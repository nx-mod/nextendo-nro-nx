// nextendo-nx — Nintendo Switch homebrew for the Nextendo Network.
// Copyright (C) 2026 Nextendo Network. PolyForm Shield License 1.0.0.
// Required Notice: Copyright 2026 Nextendo Network
//
// Reviewed C++ port of the KEPT core of Prelude's nextendo_apply.c. The trimmed
// features (SSBU/SMB35 mods, flag installer, BCAT data, S3 wizard, account-link
// UI) are intentionally absent — the servers now own that. Behaviour of what
// remains matches the audited original: hosts write, INI toggle, per-mode
// PRODINFO, cert-patch provisioning, backup, reboot.
//
// NOT COMPILED in the authoring environment (no devkitPro). Logic-reviewed only.
#include "nextendo/apply.hpp"
#include "nextendo/hosts.hpp"
#include "nextendo/config.hpp"

#include <cstdio>
#include <cstring>
#include <string>
#include <sys/stat.h>
#include <dirent.h>
#include <errno.h>
#include <unistd.h>
#include <switch.h>

namespace nextendo::apply {
namespace cfg = nextendo::config;

// ---------------------------------------------------------------- fs helpers
namespace {

Mutex g_traceMtx;

// mkdir -p (fsdev's mkdir does not create intermediate dirs). Accepts "sdmc:/…".
bool ensureDir(const std::string &path) {
    std::string tmp = path;
    size_t start = tmp.find(':');
    start = (start == std::string::npos) ? 0 : start + 1;
    if (start < tmp.size() && tmp[start] == '/') start++;
    for (size_t i = start; i <= tmp.size(); ++i) {
        if (i == tmp.size() || tmp[i] == '/') {
            if (i == tmp.size() && (i == 0 || tmp[i - 1] == '/')) break;
            char saved = (i < tmp.size()) ? tmp[i] : '\0';
            tmp[i] = '\0';
            if (mkdir(tmp.c_str(), 0777) != 0 && errno != EEXIST) return false;
            if (i < tmp.size()) tmp[i] = saved;
        }
    }
    return true;
}

bool fileExists(const std::string &path) {
    struct stat st;
    return stat(path.c_str(), &st) == 0;
}

std::string readFile(const std::string &path) {
    std::string out;
    FILE *f = fopen(path.c_str(), "rb");
    if (!f) return out;
    char buf[8192];
    size_t n;
    while ((n = fread(buf, 1, sizeof(buf), f)) > 0) out.append(buf, n);
    fclose(f);
    return out;
}

bool fileHas(const std::string &path, const std::string &needle) {
    return readFile(path).find(needle) != std::string::npos;
}

bool writeTextFile(const std::string &path, const std::string &text) {
    FILE *f = fopen(path.c_str(), "wb");
    if (!f) return false;
    bool ok = fwrite(text.data(), 1, text.size(), f) == text.size();
    fclose(f);
    return ok;
}

bool copyFileRaw(const std::string &src, const std::string &dst) {
    FILE *in = fopen(src.c_str(), "rb");
    if (!in) return false;
    FILE *out = fopen(dst.c_str(), "wb");
    if (!out) { fclose(in); return false; }
    char buf[16384];
    size_t n; bool ok = true;
    while ((n = fread(buf, 1, sizeof(buf), in)) > 0)
        if (fwrite(buf, 1, n, out) != n) { ok = false; break; }
    fclose(in); fclose(out);
    return ok;
}

// Recursive romfs -> SD mirror; overwrites (patches are build-id gated and
// idempotent, so a .nro update propagates the latest patch set).
bool copyTree(const std::string &srcDir, const std::string &dstDir) {
    DIR *d = opendir(srcDir.c_str());
    if (!d) return false;
    bool ok = true;
    for (struct dirent *e; (e = readdir(d)) != nullptr;) {
        if (!strcmp(e->d_name, ".") || !strcmp(e->d_name, "..")) continue;
        std::string sp = srcDir + "/" + e->d_name;
        std::string dp = dstDir + "/" + e->d_name;
        struct stat st;
        if (stat(sp.c_str(), &st) == 0 && S_ISDIR(st.st_mode)) {
            if (!ensureDir(dp) || !copyTree(sp, dp)) { ok = false; break; }
        } else if (!copyFileRaw(sp, dp)) { ok = false; break; }
    }
    closedir(d);
    return ok;
}

// Recursive romfs-mirrored removal: delete every SD path that the romfs tree
// names (used to strip the cert-trust stack in Nintendo mode).
bool removeTree(const std::string &srcDir, const std::string &dstBase) {
    DIR *d = opendir(srcDir.c_str());
    if (!d) return true; // nothing to mirror
    for (struct dirent *e; (e = readdir(d)) != nullptr;) {
        if (!strcmp(e->d_name, ".") || !strcmp(e->d_name, "..")) continue;
        std::string sp = srcDir + "/" + e->d_name;
        std::string dp = dstBase + "/" + e->d_name;
        struct stat st;
        if (stat(sp.c_str(), &st) == 0 && S_ISDIR(st.st_mode)) {
            removeTree(sp, dp);
            rmdir(dp.c_str());
        } else {
            remove(dp.c_str());
        }
    }
    closedir(d);
    return true;
}

// Set `key = value` inside [section] of an INI file, creating the section/key if
// absent and replacing a commented-out or existing key in place. Faithful port
// of Prelude's iniSetDnsMitm/iniSetBlankProdinfoEmummc, generalised.
bool iniSetKey(const std::string &path, const char *section,
               const char *key, const std::string &fullLine) {
    std::string in = readFile(path);
    std::string out;
    out.reserve(in.size() + fullLine.size() + 32);
    const std::string secTag = std::string("[") + section + "]";

    bool inSec = false, sawSec = false, setK = false;
    size_t pos = 0;
    while (pos < in.size()) {
        size_t nl = in.find('\n', pos);
        std::string line = in.substr(pos, nl == std::string::npos ? std::string::npos : nl - pos + 1);
        pos = (nl == std::string::npos) ? in.size() : nl + 1;

        size_t t = line.find_first_not_of(" \t");
        if (t != std::string::npos && line[t] == '[') {
            if (inSec && !setK) { out += fullLine; setK = true; }
            inSec = line.compare(t, secTag.size(), secTag) == 0;
            if (inSec) sawSec = true;
            out += line;
        } else if (inSec) {
            size_t k = (t == std::string::npos) ? line.size() : t;
            if (k < line.size() && (line[k] == ';' || line[k] == '#')) {
                k++;
                while (k < line.size() && (line[k] == ' ' || line[k] == '\t')) k++;
            }
            if (line.compare(k, strlen(key), key) == 0) { out += fullLine; setK = true; }
            else out += line;
        } else {
            out += line;
        }
    }
    if (inSec && !setK) {
        if (!out.empty() && out.back() != '\n') out += '\n';
        out += fullLine; setK = true;
    }
    if (!sawSec) {
        if (!out.empty() && out.back() != '\n') out += '\n';
        out += secTag; out += '\n';
        out += fullLine;
    }
    return writeTextFile(path, out);
}

bool setDnsMitm(bool enable, bool addDefaults) {
    ensureDir(cfg::kSettingsDir);
    bool a = iniSetKey(cfg::kSettingsIni, "atmosphere", "enable_dns_mitm",
                       enable ? "enable_dns_mitm = u8!0x1\n" : "enable_dns_mitm = u8!0x0\n");
    bool b = iniSetKey(cfg::kSettingsIni, "atmosphere", "add_defaults_to_dns_hosts",
                       addDefaults ? "add_defaults_to_dns_hosts = u8!0x1\n"
                                   : "add_defaults_to_dns_hosts = u8!0x0\n");
    return a && b;
}

// emuMMC PRODINFO: real under Nextendo (valid device cert, confined by DNS.mitm
// so identity never leaks to Nintendo), blanked under Nintendo (white identity
// if the user visits real Nintendo on emuMMC). sysNAND PRODINFO is never touched.
bool setBlankProdinfoEmummc(bool blank) {
    return iniSetKey(cfg::kExosphereIni, "exosphere", "blank_prodinfo_emummc",
                     blank ? "blank_prodinfo_emummc=1\n" : "blank_prodinfo_emummc=0\n");
}

// Remove trace/log files that could leak the server IP.
void purgeLeaks() {
    remove(cfg::kTracePath);
}

} // namespace

// ---------------------------------------------------------------- public API

void trace(const std::string &step) {
    mutexLock(&g_traceMtx);
    ensureDir(cfg::kBackupDir); // trace lives under our switch/ dir
    FILE *f = fopen(cfg::kTracePath, "a");
    if (f) { fputs(step.c_str(), f); fputc('\n', f); fclose(f); }
    fsdevCommitDevice("sdmc"); // durable immediately — the whole point of a trace
    mutexUnlock(&g_traceMtx);
}

Boot detectBoot() {
    if (R_FAILED(splInitialize())) return Boot::Unknown;
    u64 val = 0;
    Result rc = splGetConfig((SplConfigItem)65007, &val); // ExosphereEmummcType
    splExit();
    if (R_FAILED(rc)) return Boot::Unknown;
    return (val != 0) ? Boot::Emummc : Boot::Sysmmc;
}

Mode currentMode() {
    for (const char *p : {cfg::kHostsSysmmc, cfg::kHostsEmummc})
        for (const auto &s : cfg::kServers)
            if (fileHas(p, s.ip)) return Mode::Nextendo;
    return Mode::Nintendo;
}

bool applyNextendo(const std::string &ip) {
    if (!ensureDir(cfg::kHostsDir)) return false;

    // Provision the cert-trust patch set (romfs:/sd -> sdmc:). Without it the
    // console does not trust our TLS and online entry fails.
    if (!copyTree(cfg::kProvisionSrc, cfg::kProvisionDst)) {
        trace("WARN: cert-patch provisioning failed -> abort");
        return false;
    }

    std::string body = nextendo::hosts::build(ip);

    // Back up the user's own default.txt once before we overwrite it.
    if (fileExists(cfg::kHostsDefault) && !fileExists(cfg::kBackupDefault)) {
        ensureDir(cfg::kBackupDir);
        copyFileRaw(cfg::kHostsDefault, cfg::kBackupDefault);
    }

    bool a = writeTextFile(cfg::kHostsSysmmc, body);
    bool b = writeTextFile(cfg::kHostsEmummc, body);
    bool c = writeTextFile(cfg::kHostsDefault, body);
    if (!c) trace("WARN: default.txt not written");

    // add_defaults_to_dns_hosts=0 in NEXTENDO mode: the native table redirects
    // receive-%.dg/er to 127.0.0.1, which would re-block the telemetry we just
    // routed to our sink and hang the NSO applet. Our two receive-% lines keep
    // telemetry blocked (to our sink), so protection is preserved.
    bool i = setDnsMitm(true, false);
    bool p = setBlankProdinfoEmummc(false);
    if (!p) trace("WARN: blank_prodinfo_emummc(false) failed -> risk 2123-0011");

    purgeLeaks();
    fsdevCommitDevice("sdmc");
    return a && b && c && i && p;
}

bool applyNintendo() {
    trace("apply_nintendo: enter");
    // Delete our hosts (Nextendo mode rewrites them from scratch).
    remove(cfg::kHostsSysmmc);
    remove(cfg::kHostsEmummc);
    // default.txt: only delete it if it is OURS (carries our marker); a user's
    // own default.txt is not ours to remove.
    if (fileHas(cfg::kHostsDefault, cfg::kHostsMarker))
        remove(cfg::kHostsDefault);
    // Restore the user's pre-Prelude default.txt if we backed one up.
    if (fileExists(cfg::kBackupDefault))
        copyFileRaw(cfg::kBackupDefault, cfg::kHostsDefault);

    // Strip the cert-trust stack (disable_ca_verification patches, our CA, etc.):
    // it must not survive into Nintendo mode or the whole system stays MITM-able.
    // Reversible: applyNextendo re-provisions it.
    removeTree(cfg::kProvisionSrc, cfg::kProvisionDst);

    // enable_dns_mitm=0, and add_defaults_to_dns_hosts=1 so Atmosphère's native
    // telemetry blocking (compiled into DNS.mitm) applies from boot even with an
    // empty /atmosphere/hosts. blank_prodinfo_emummc=1 keeps a white identity if
    // the user visits real Nintendo on emuMMC.
    bool i = setDnsMitm(false, true);
    bool p = setBlankProdinfoEmummc(true);

    purgeLeaks();
    fsdevCommitDevice("sdmc");
    trace("apply_nintendo: done");
    return i && p;
}

Result reboot() {
    Result rc = bpcInitialize();
    if (R_FAILED(rc)) return rc;
    rc = bpcRebootSystem(); // does not return on success
    bpcExit();
    return rc;
}

} // namespace nextendo::apply
