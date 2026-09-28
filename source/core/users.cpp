// nextendo-nx — the console's users.
// Copyright (C) 2026 Nextendo Network. PolyForm Shield License 1.0.0.
//
// Uses the account service's administrator interface (acc:su), as documented on switchbrew:
//   acc:su 250                 GetBaasAccountAdministrator(uid) -> IAdministrator
//   IAdministrator 250         IsLinkedWithNintendoAccount -> bool
//   IAdministrator 203         DeleteRegistrationInfoLocally
#include "nextendo/users.hpp"

#include <cstdio>
#include <cstring>

namespace nextendo::users {

static std::string hexResult(Result rc) {
    char b[32];
    std::snprintf(b, sizeof b, "0x%X (%04u-%04u)", rc, 2000 + R_MODULE(rc), R_DESCRIPTION(rc));
    return b;
}

static Result baasAdministrator(const AccountUid &uid, Service *out) {
    return serviceDispatchIn(accountGetServiceSession(), 250, uid, .out_num_objects = 1, .out_objects = out);
}

static bool isLinked(const AccountUid &uid) {
    Service admin;
    bool linked = false;
    if (R_SUCCEEDED(baasAdministrator(uid, &admin))) {
        if (R_FAILED(serviceDispatchOut(&admin, 250, linked))) linked = false;
        serviceClose(&admin);
    }
    return linked;
}

std::vector<User> list(std::string &err) {
    std::vector<User> out;
    Result rc = accountInitialize(AccountServiceType_Administrator);
    if (R_FAILED(rc)) {
        err = "acc:su " + hexResult(rc);
        return out;
    }
    AccountUid uids[ACC_USER_LIST_SIZE];
    s32 count = 0;
    rc = accountListAllUsers(uids, ACC_USER_LIST_SIZE, &count);
    if (R_FAILED(rc)) {
        err = "list users " + hexResult(rc);
        accountExit();
        return out;
    }
    for (s32 i = 0; i < count; i++) {
        User u{uids[i], "?", false};
        AccountProfile profile;
        AccountProfileBase base{};
        if (R_SUCCEEDED(accountGetProfile(&profile, uids[i]))) {
            if (R_SUCCEEDED(accountProfileGet(&profile, nullptr, &base)))
                u.nickname = std::string(base.nickname, strnlen(base.nickname, sizeof base.nickname));
            accountProfileClose(&profile);
        }
        u.linked = isLinked(uids[i]);
        out.push_back(u);
    }
    accountExit();
    return out;
}

bool unlinkLocally(const AccountUid &uid, std::string &err) {
    Result rc = accountInitialize(AccountServiceType_Administrator);
    if (R_FAILED(rc)) {
        err = "acc:su " + hexResult(rc);
        return false;
    }
    Service admin;
    rc = baasAdministrator(uid, &admin);
    if (R_SUCCEEDED(rc)) {
        rc = serviceDispatch(&admin, 203);
        serviceClose(&admin);
    }
    accountExit();
    if (R_FAILED(rc)) {
        err = hexResult(rc);
        return false;
    }
    return true;
}

} // namespace nextendo::users
