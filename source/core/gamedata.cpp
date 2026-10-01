// nextendo-nx — the installed games' BCAT (delivery cache) settings.
// Copyright (C) 2026 Nextendo Network. PolyForm Shield License 1.0.0.
#include "nextendo/gamedata.hpp"

#include <switch.h>
#include <sys/stat.h>
#include <cstdio>
#include <cstring>
#include <memory>

namespace nextendo::gamedata {

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
            const char *name = "?";
            for (const NacpLanguageEntry &e : nacp.lang) {
                if (e.name[0]) {
                    name = e.name;
                    break;
                }
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
