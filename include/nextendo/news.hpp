// nextendo-nx — the console's news (HOME menu News).
// Copyright (C) 2026 Nextendo Network. PolyForm Shield License 1.0.0.
#pragma once
#include <string>

namespace nextendo::news {

// Copies the console's news records to sd:/switch/nextendo-nx/news/: each item's data (msgpack) as
// <news_id>.msgpack and its database row in records.txt. Read-only on the console. This is the reference
// for the format local Nextendo news must use. Returns how many items were written, or -1 with `err` set.
int dump(std::string &err);

// Posts every news record in sd:/switch/nextendo-nx/news/post/*.msgpack as local news (PostLocalNews: no
// network, no signature). Each result goes to news/post.txt. Returns how many were accepted, or -1 with `err`.
int post(std::string &err);

// Removes every news item from the console (ClearStorage: Nintendo's built-in notices too), then subscribes
// the default channels again (clearing drops the subscriptions). Dump first to be able to bring items back
// with post(). Returns true on success, else false with `err`.
bool clear(std::string &err);

// Subscribes the default channels (nx_news, nx_notice, nx_news_nextendo) where they are not already, and asks
// the console to fetch them from the server now. Returns a line per channel (its old -> new subscription
// status), or "" with `err` set. Every call's result goes to sd:/switch/nextendo-nx/news/subscribe.txt.
// A diagnostic (the Subscribe button): the servers subscribe consoles and tell them when to fetch.
std::string fetch(std::string &err);

} // namespace nextendo::news
