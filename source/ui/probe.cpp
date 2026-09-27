// nextendo-nx — LAN server reachability probe.
// Copyright (C) 2026 Nextendo Network. PolyForm Shield License 1.0.0.
#include "ui/probe.hpp"

#include <cstring>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

namespace ui::probe {

bool reachable(const std::string &ip, int port, int timeoutMs) {
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0)
        return false;

    // Non-blocking connect + select, so a dead host times out fast.
    int flags = fcntl(fd, F_GETFL, 0);
    fcntl(fd, F_SETFL, flags | O_NONBLOCK);

    sockaddr_in addr {};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(static_cast<uint16_t>(port));
    if (inet_pton(AF_INET, ip.c_str(), &addr.sin_addr) != 1) {
        close(fd);
        return false;
    }

    bool ok = false;
    int rc = connect(fd, reinterpret_cast<sockaddr *>(&addr), sizeof(addr));
    if (rc == 0) {
        ok = true;
    } else {
        fd_set wset;
        FD_ZERO(&wset);
        FD_SET(fd, &wset);
        timeval tv { timeoutMs / 1000, (timeoutMs % 1000) * 1000 };
        if (select(fd + 1, nullptr, &wset, nullptr, &tv) > 0) {
            int err = 0;
            socklen_t len = sizeof(err);
            if (getsockopt(fd, SOL_SOCKET, SO_ERROR, &err, &len) == 0 && err == 0)
                ok = true;
        }
    }
    close(fd);
    return ok;
}

} // namespace ui::probe
