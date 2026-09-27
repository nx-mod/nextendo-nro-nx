// nextendo-nx — LAN server reachability probe.
// Copyright (C) 2026 Nextendo Network. PolyForm Shield License 1.0.0.
#pragma once
#include <string>

namespace ui::probe {
// Try a TCP connect to ip:port with a timeout; true if the port accepts. Used by
// the Diagnostics pane to tell the user, on their own LAN, whether the Nextendo
// servers are up — the thing you actually want to know when something's off.
bool reachable(const std::string &ip, int port, int timeoutMs = 800);
}
