// nextendo-nx — the installed games' BCAT (delivery cache) settings.
// Copyright (C) 2026 Nextendo Network. PolyForm Shield License 1.0.0.
#include "nextendo/gamedata.hpp"

#include <switch.h>
#include <sys/stat.h>
#include <cstdio>
#include <cstring>
#include <memory>

namespace nextendo::gamedata {

// validUtf8 is true if s[0..len) is well-formed UTF-8 with no C0/C1 control bytes (except none are expected in
// a name). Rejects the raw binary that some games leave in the control-name buffer.
static bool validUtf8(const char *s, size_t len) {
    for (size_t i = 0; i < len;) {
        unsigned char c = (unsigned char)s[i];
        size_t n; // continuation bytes
        if (c < 0x20) return false;
        else if (c < 0x80) n = 0;
        else if ((c & 0xE0) == 0xC0 && c >= 0xC2) n = 1;
        else if ((c & 0xF0) == 0xE0) n = 2;
        else if ((c & 0xF8) == 0xF0 && c <= 0xF4) n = 3;
        else return false;
        if (i + n >= len && n > 0 && i + n + 1 > len) return false;
        for (size_t k = 1; k <= n; k++)
            if (i + k >= len || ((unsigned char)s[i + k] & 0xC0) != 0x80) return false;
        i += n + 1;
    }
    return true;
}

int dump(int &total, std::string &err) {
    total = 0;
    Result rc = nsInitialize();
    if (R_FAILED(rc)) {
        char b[48];
        std::snprintf(b, sizeof b, "ns 0x%X", rc);
        err = b;
        return -1;
    }
    mkdir("sdmc:/switch/nextendo-nx/bcat", 0777);
    FILE *titles = std::fopen("sdmc:/switch/nextendo-nx/bcat/titles.txt", "w");
    FILE *pass = std::fopen("sdmc:/switch/nextendo-nx/bcat/passphrases.txt", "w");
    if (!titles || !pass) {
        if (titles) std::fclose(titles);
        if (pass) std::fclose(pass);
        nsExit();
        err = "cannot write sd:/switch/nextendo-nx/bcat/";
        return -1;
    }

    // The control data (NACP + icon) is large: keep it off the stack.
    auto data = std::make_unique<NsApplicationControlData>();
    int withBcat = 0;
    for (s32 offset = 0;;) {
        NsApplicationRecord records[32];
        s32 n = 0;
        if (R_FAILED(nsListApplicationRecord(records, 32, offset, &n)) || n <= 0) break;
        offset += n;
        for (s32 i = 0; i < n; i++) {
            const u64 id = records[i].application_id;
            u64 size = 0;
            rc = nsGetApplicationControlData(NsApplicationControlSource_Storage, id, data.get(), sizeof(*data), &size);
            if (R_FAILED(rc) || size < sizeof(data->nacp)) {
                std::fprintf(titles, "%016lx  (no control data: 0x%X)\n", id, rc);
                total++;
                continue;
            }
            const NacpStruct &nacp = data->nacp;
            // The control name is a fixed buffer, not always NUL-terminated and sometimes not valid text:
            // take the first entry that is valid UTF-8 with no control bytes (so accented names like
            // "Pokémon" show, but binary garbage is rejected).
            char name[sizeof nacp.lang[0].name + 1] = "?";
            for (const NacpLanguageEntry &e : nacp.lang) {
                const size_t len = strnlen(e.name, sizeof e.name);
                if (len == 0 || !validUtf8(e.name, len)) continue;
                std::memcpy(name, e.name, len);
                name[len] = '\0';
                break;
            }
            const size_t passLen = strnlen(nacp.bcat_passphrase, sizeof nacp.bcat_passphrase);
            const bool uses = nacp.bcat_delivery_cache_storage_size != 0 || passLen != 0;
            std::fprintf(titles, "%016lx  cache %8lu KiB  passphrase %s  %s\n", id,
                         nacp.bcat_delivery_cache_storage_size / 1024, passLen ? "yes" : "no ", name);
            if (passLen) std::fprintf(pass, "%016lx=%.*s\n", id, (int)passLen, nacp.bcat_passphrase);
            total++;
            if (uses) withBcat++;
        }
    }
    std::fclose(titles);
    std::fclose(pass);
    nsExit();
    return withBcat;
}

} // namespace nextendo::gamedata
