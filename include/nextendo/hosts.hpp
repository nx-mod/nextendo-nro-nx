// nextendo-nx — Nintendo Switch homebrew for the Nextendo Network.
// Copyright (C) 2026 Nextendo Network. PolyForm Shield License 1.0.0.
// Required Notice: Copyright 2026 Nextendo Network
#pragma once
#include <string>

namespace nextendo::hosts {

// Build the Atmosphère DNS.mitm hosts-file body for NEXTENDO mode, pointing the
// Nintendo domains at `ip`. The returned text is written verbatim to
// sysmmc.txt / emummc.txt / default.txt.
//
// Invariants preserved from the audited production list (do not reorder):
//   * "last matching line wins" — the MK8 nncs2 override sits AFTER the g2* wildcard.
//   * nncs2 uses a DISTINCT IP from nncs1 (Pia dedups identical probes -> 2618-201).
//   * hosts that end in .nintendo.net but NOT srv.nintendo.net (dragons, lp1.nso,
//     *.npln, gamesync, bcat *.cdn) are named explicitly; no wildcard covers them.
//   * d4c is NOT redirected (nim stores a persistent flag).
//   * telemetry (receive-%.dg/er) points at our sink, never a black hole, or the
//     NSO applet hangs waiting for a synchronous response.
std::string build(const std::string &ip);

} // namespace nextendo::hosts
