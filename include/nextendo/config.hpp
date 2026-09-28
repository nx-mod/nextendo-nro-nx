// nextendo-nx — Nintendo Switch homebrew for the Nextendo Network.
// Copyright (C) 2026 Nextendo Network. PolyForm Shield License 1.0.0.
// Required Notice: Copyright 2026 Nextendo Network
#pragma once
#include <string>
#include <array>

// Central server + path configuration. Every other file pulls addresses and SD
// paths from here instead of hard-coding them. Override the default server at
// build time with -DNEXTENDO_SERVER_IP_DEFAULT=\"a.b.c.d\".
namespace nextendo::config {

#ifndef NEXTENDO_SERVER_IP_DEFAULT
#define NEXTENDO_SERVER_IP_DEFAULT "51.178.29.194"
#endif

// The DNS.mitm redirect targets. `nncs2` MUST differ from the main IP: Pia
// dedups identical nncs1/nncs2 probes and never sends the second, so NAT never
// completes and MK8/Splatoon fall into error 2618-201.
// nx-mod: `make LAN_HOST=<ip> [LAN_HOST2=<ip>]` points every address at a nextendo-testing LAN stack
// (nncs2 on LAN_HOST2, MK8 on the stack too); the production VPS stays the alternative.
#ifndef NEXTENDO_SERVER_IP_NNCS2
#define NEXTENDO_SERVER_IP_NNCS2 "164.132.111.120"
#endif
#ifndef NEXTENDO_SERVER_IP_MK8
#define NEXTENDO_SERVER_IP_MK8 "164.132.111.120"
#endif

inline constexpr const char *kServerDefault = NEXTENDO_SERVER_IP_DEFAULT;
#ifdef NEXTENDO_LAN
inline constexpr const char *kServerAlt     = "51.178.29.194";
#else
inline constexpr const char *kServerAlt     = "3.135.232.168";
#endif
inline constexpr const char *kServerNncs2   = NEXTENDO_SERVER_IP_NNCS2;
// MK8 production secure-server (players are here; the dev VPS has none). This is
// a per-host override that must sit AFTER the g2* wildcard in the hosts file.
inline constexpr const char *kServerMk8Prod = NEXTENDO_SERVER_IP_MK8;

struct ServerChoice { const char *ip; const char *label; };
inline constexpr std::array<ServerChoice, 2> kServers{{
#ifdef NEXTENDO_LAN
    {kServerDefault, "LAN stack (nextendo-testing)"},
    {kServerAlt,     "Nextendo (production)"},
#else
    {kServerDefault, "VPS (production)"},
    {kServerAlt,     "Alternate / local"},
#endif
}};

// SD-card paths (Atmosphère reads these at boot).
inline constexpr const char *kHostsDir      = "sdmc:/atmosphere/hosts";
inline constexpr const char *kHostsSysmmc   = "sdmc:/atmosphere/hosts/sysmmc.txt";
inline constexpr const char *kHostsEmummc   = "sdmc:/atmosphere/hosts/emummc.txt";
inline constexpr const char *kHostsDefault  = "sdmc:/atmosphere/hosts/default.txt";
inline constexpr const char *kSettingsDir   = "sdmc:/atmosphere/config";
inline constexpr const char *kSettingsIni   = "sdmc:/atmosphere/config/system_settings.ini";
inline constexpr const char *kExosphereIni  = "sdmc:/exosphere.ini";

inline constexpr const char *kBackupDir     = "sdmc:/switch/nextendo-nx/backup";
inline constexpr const char *kBackupDefault = "sdmc:/switch/nextendo-nx/backup/default.txt";
inline constexpr const char *kConfigPath    = "sdmc:/switch/nextendo-nx/config.ini";
inline constexpr const char *kTracePath     = "sdmc:/switch/nextendo-nx/trace.txt";

// Marker written into our hosts files so we can recognise them regardless of the
// current server IP.
inline constexpr const char *kHostsMarker   = "NEXTENDO NETWORK - Atmosphere DNS-MITM";

// romfs -> SD provisioning root (cert-trust patch set + optional sysmodules).
inline constexpr const char *kProvisionSrc  = "romfs:/sd";
inline constexpr const char *kProvisionDst  = "sdmc:";

} // namespace nextendo::config
