// nextendo-nx — Nintendo Switch homebrew for the Nextendo Network.
// Copyright (C) 2026 Nextendo Network. PolyForm Shield License 1.0.0.
// Required Notice: Copyright 2026 Nextendo Network
#pragma once
#include <string>
#include <switch.h>

// System logic: write the DNS.mitm hosts, toggle enable_dns_mitm in
// system_settings.ini, set per-mode PRODINFO in exosphere.ini, provision the
// cert-trust patch set, and reboot. This is the irreducible core of the app —
// everything the old Prelude did around game-specific mods/BCAT/flags is gone;
// the servers now handle that (see RESTRUCTURE.md).
namespace nextendo::apply {

enum class Mode { Nextendo = 0, Nintendo = 1 };
enum class Boot { Unknown = -1, Sysmmc = 0, Emummc = 1 };

// Append a step to the trace file and commit immediately, so the last line on
// the card is the last step reached even after a forced power-off.
void trace(const std::string &step);

// Detect boot storage (cosmetic — we write both hosts files regardless).
Boot detectBoot();

// Which mode is currently loaded, read from the hosts files on the SD.
Mode currentMode();

// NEXTENDO mode: provision cert patches, write hosts (sysmmc/emummc/default),
// enable_dns_mitm=1, real PRODINFO on emuMMC. Returns false on failure.
bool applyNextendo(const std::string &ip);

// NINTENDO mode: remove our hosts, restore the user's default.txt if backed up,
// enable_dns_mitm=0, blanked PRODINFO on emuMMC, strip the cert-trust stack, and
// purge logs that could leak the server IP. Native Atmosphère telemetry blocking
// is restored (add_defaults_to_dns_hosts=1).
bool applyNintendo();

// Reboot (does not return on success; bpcRebootSystem).
Result reboot();

} // namespace nextendo::apply
