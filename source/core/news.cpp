// nextendo-nx — the console's news (HOME menu News).
// Copyright (C) 2026 Nextendo Network. PolyForm Shield License 1.0.0.
#include "nextendo/news.hpp"

#include <switch.h>
#include <sys/stat.h>
#include <dirent.h>
#include <cstdio>
#include <cstring>
#include <string>
#include <vector>

namespace nextendo::news {

static const char *kDir = "sdmc:/switch/nextendo-nx/news";

static std::string hexResult(Result rc) {
    char b[32];
    std::snprintf(b, sizeof b, "0x%X (%04u-%04u)", rc, 2000 + R_MODULE(rc), R_DESCRIPTION(rc));
    return b;
}

int dump(std::string &err) {
    mkdir("sdmc:/switch", 0777);
    mkdir("sdmc:/switch/nextendo-nx", 0777);
    mkdir(kDir, 0777);

    Result rc = newsInitialize(NewsServiceType_Administrator);
    if (R_FAILED(rc)) {
        err = "news:a " + hexResult(rc);
        return -1;
    }
    NewsDatabaseService db;
    rc = newsCreateNewsDatabaseService(&db);
    if (R_FAILED(rc)) {
        err = "news database " + hexResult(rc);
        newsExit();
        return -1;
    }

    std::vector<NewsRecord> records(32);
    u32 count = 0;
    rc = newsDatabaseGetList(&db, records.data(), records.size(), "", "received_at DESC", &count, 0);
    if (R_FAILED(rc)) {
        err = "news list " + hexResult(rc);
        newsDatabaseClose(&db);
        newsExit();
        return -1;
    }

    std::string index = std::string(kDir) + "/records.txt";
    FILE *idx = std::fopen(index.c_str(), "w");
    int written = 0;
    for (u32 i = 0; i < count; i++) {
        NewsRecord &r = records[i];
        std::string id(r.news_id, strnlen(r.news_id, sizeof r.news_id));
        if (idx) {
            std::fprintf(idx, "news_id=%s user_id=%.*s topic_id=%.*s received_at=%lld decoration_type=%d read=%d newly=%d displayed=%d feedback=%d extra_1=%d extra_2=%d\n",
                         id.c_str(), (int)sizeof r.user_id, r.user_id, (int)sizeof r.topic_id.name, r.topic_id.name,
                         (long long)r.received_at, r.decoration_type, r.read, r.newly, r.displayed, r.feedback, r.extra_1, r.extra_2);
        }
        // The record first; then by file name, in the forms the news save is known to use.
        NewsDataService data;
        Result orc = newsCreateNewsDataService(&data); // a data session per item, opened below
        if (R_FAILED(orc)) {
            if (idx) std::fprintf(idx, "  create data service: %s\n", hexResult(orc).c_str());
            continue;
        }
        orc = newsDataOpenWithNewsRecord(&data, &r);
        std::string how = "record";
        const std::string names[] = {id, id + ".msgpack", "D00000000000000000000_" + id + ".msgpack", "data/" + id + ".msgpack"};
        for (const auto &n : names) {
            if (R_SUCCEEDED(orc)) break;
            if (idx) std::fprintf(idx, "  open %s: %s\n", how.c_str(), hexResult(orc).c_str());
            how = n;
            orc = newsDataOpen(&data, n.c_str());
        }
        if (R_FAILED(orc)) {
            if (idx) std::fprintf(idx, "  open %s: %s\n", how.c_str(), hexResult(orc).c_str());
            newsDataClose(&data);
            continue;
        }
        if (idx) std::fprintf(idx, "  opened with %s\n", how.c_str());
        u64 size = 0;
        if (R_SUCCEEDED(newsDataGetSize(&data, &size)) && size > 0 && size < 4 * 1024 * 1024) {
            std::vector<u8> buf(size);
            u64 got = 0;
            if (R_SUCCEEDED(newsDataRead(&data, &got, 0, buf.data(), buf.size())) && got > 0) {
                std::string path = std::string(kDir) + "/" + (id.empty() ? std::to_string(i) : id) + ".msgpack";
                if (FILE *f = std::fopen(path.c_str(), "wb")) {
                    std::fwrite(buf.data(), 1, got, f);
                    std::fclose(f);
                    written++;
                }
            }
        }
        newsDataClose(&data);
    }
    if (idx) std::fclose(idx);
    fsdevCommitDevice("sdmc");
    newsDatabaseClose(&db);
    newsExit();
    return written;
}

int post(std::string &err) {
    const std::string dir = std::string(kDir) + "/post";
    DIR *d = opendir(dir.c_str());
    if (!d) {
        err = "no " + dir.substr(5) + " folder";
        return -1;
    }
    Result rc = newsInitialize(NewsServiceType_Administrator);
    if (R_FAILED(rc)) {
        closedir(d);
        err = "news:a " + hexResult(rc);
        return -1;
    }
    FILE *log = std::fopen((std::string(kDir) + "/post.txt").c_str(), "w");
    int ok = 0;
    while (struct dirent *e = readdir(d)) {
        std::string name = e->d_name;
        if (name.size() < 8 || name.compare(name.size() - 8, 8, ".msgpack") != 0) continue;
        FILE *f = std::fopen((dir + "/" + name).c_str(), "rb");
        if (!f) continue;
        std::vector<u8> buf;
        u8 chunk[4096];
        size_t n;
        while ((n = std::fread(chunk, 1, sizeof chunk, f)) > 0) buf.insert(buf.end(), chunk, chunk + n);
        std::fclose(f);
        rc = newsPostLocalNews(buf.data(), buf.size());
        if (log) std::fprintf(log, "%s (%zu bytes): %s\n", name.c_str(), buf.size(), R_SUCCEEDED(rc) ? "posted" : hexResult(rc).c_str());
        if (R_SUCCEEDED(rc)) ok++;
    }
    closedir(d);
    if (log) std::fclose(log);
    fsdevCommitDevice("sdmc");
    newsExit();
    return ok;
}

// The HOME menu's default channels, and the status nn::news uses for "subscribed".
static const char *const kTopics[] = {"nx_news", "nx_notice", "nx_news_nextendo"};
static constexpr u32 kSubscribed = 2;
static constexpr u32 kAutoSubscribed = 3;

// Subscribes kTopics and requests their reception now. The news service must be initialised.
static std::string subscribeDefaults() {
    std::string out, log;
    for (const char *t : kTopics) {
        u32 before = 0, after = 0;
        Result getRc = newsGetSubscriptionStatus(t, &before);
        // 2 is subscribed (what the HOME menu writes on Follow), 3 the system's own auto-subscription: leave both.
        const bool subscribed = R_SUCCEEDED(getRc) && (before == kSubscribed || before == kAutoSubscribed);
        Result setRc = subscribed ? 0 : newsSetSubscriptionStatus(t, kSubscribed);
        Result reqRc = R_SUCCEEDED(setRc) ? newsRequestImmediateReception(t) : setRc;
        newsGetSubscriptionStatus(t, &after);
        char line[160];
        if (R_SUCCEEDED(setRc) && R_SUCCEEDED(reqRc))
            std::snprintf(line, sizeof line, "%s: %u -> %u\n", t, before, after);
        else
            std::snprintf(line, sizeof line, "%s: %s\n", t, hexResult(R_FAILED(setRc) ? setRc : reqRc).c_str());
        out += line;
        std::snprintf(line, sizeof line, "%s before=%u (get 0x%X) set(%u)=0x%X request=0x%X after=%u\n", t, before,
                      getRc, kSubscribed, setRc, reqRc, after);
        log += line;
    }
    // Every call's result, for diagnosis (sd:/switch/nextendo-nx/news/subscribe.txt).
    NewsTopicName topics[32];
    u32 n = 0;
    for (u32 channel = 0; channel < 3; channel++) {
        Result rc = newsGetTopicList(channel, &n, topics, 32);
        char line[64];
        std::snprintf(line, sizeof line, "topic list %u: 0x%X, %u topic(s)\n", channel, rc, R_SUCCEEDED(rc) ? n : 0);
        log += line;
        for (u32 i = 0; R_SUCCEEDED(rc) && i < n && i < 32; i++)
            log += "  " + std::string(topics[i].name, strnlen(topics[i].name, sizeof topics[i].name)) + "\n";
    }
    mkdir("sdmc:/switch/nextendo-nx/news", 0777);
    if (FILE *f = std::fopen("sdmc:/switch/nextendo-nx/news/subscribe.txt", "w")) {
        std::fwrite(log.data(), 1, log.size(), f);
        std::fclose(f);
    }
    return out;
}

bool clear(std::string &err) {
    Result rc = newsInitialize(NewsServiceType_Administrator);
    if (R_FAILED(rc)) {
        err = "news:a " + hexResult(rc);
        return false;
    }
    rc = newsClearStorage();
    if (R_SUCCEEDED(rc)) subscribeDefaults(); // clearing drops the subscriptions: no news would come back
    newsExit();
    if (R_FAILED(rc)) {
        err = "clear " + hexResult(rc);
        return false;
    }
    return true;
}

std::string fetch(std::string &err) {
    Result rc = newsInitialize(NewsServiceType_Administrator);
    if (R_FAILED(rc)) {
        err = "news:a " + hexResult(rc);
        return "";
    }
    std::string out = subscribeDefaults();
    newsExit();
    return out;
}

} // namespace nextendo::news
