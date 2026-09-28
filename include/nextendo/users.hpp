// nextendo-nx — the console's users.
// Copyright (C) 2026 Nextendo Network. PolyForm Shield License 1.0.0.
#pragma once
#include <switch.h>
#include <string>
#include <vector>

namespace nextendo::users {

struct User {
    AccountUid uid;
    std::string nickname;
    bool linked = false; // linked to a Nintendo Account
};

// Every user on the console, with its link state. Empty with `err` set on failure.
std::vector<User> list(std::string &err);

// Removes the user's Nintendo Account link on this console only (the account service's
// DeleteRegistrationInfoLocally): nothing is sent to a server. A user left half-linked by a failed link can
// then be deleted in System Settings. Returns false with `err` on failure.
bool unlinkLocally(const AccountUid &uid, std::string &err);

} // namespace nextendo::users
