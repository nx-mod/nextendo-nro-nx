# nextendo-nx

**nextendo-nx** is the Nintendo Switch homebrew that flips your console between the
[Nextendo Network](https://nextendo.network) and Nintendo's official servers.

It is a small `.nro` you run from the homebrew menu. Pick a mode, it applies the
change and reboots. Nothing is permanent — switch back whenever you want.

```
        Which network do you want to load?
        [ NEXTENDO ]     [ NINTENDO ]
```

This is the trimmed, restructured successor to **Prelude**, rewritten on the
[Aether](https://github.com/tallbl0nde/Aether) GUI library (the one TriPlayer
uses). Where Prelude bundled ~14 MB of per-game mods, BCAT data and patches in the
`.nro`, nextendo-nx ships almost none of it: the network now delivers that.

## What it does

- **Nextendo mode**: writes the Atmosphère DNS.mitm hosts that redirect Nintendo
  traffic to the Nextendo servers, enables `enable_dns_mitm`, provisions the
  certificate-trust patch set, and (on emuMMC) serves the real device cert —
  confined by DNS.mitm, so your identity never leaks to Nintendo.
- **Nintendo mode**: removes every host redirect and the cert-trust stack,
  restores your own `default.txt` if it backed one up, blanks emuMMC PRODINFO, and
  restores Atmosphère's native telemetry blocking.

Telemetry is blocked in **both** modes.

## How it works

nextendo-nx does not patch games and does not touch the network stack. It writes
configuration Atmosphère already reads at boot, then reboots:

| What | Where |
| --- | --- |
| Host redirections | `/atmosphere/hosts/{sysmmc,emummc,default}.txt` |
| DNS.mitm on/off | `/atmosphere/config/system_settings.ini` |
| PRODINFO per mode | `/exosphere.ini` (emuMMC only) |
| Certificate trust | `exefs_patches/`, `nro_patches/`, CA bundle (`romfs/sd`) |

Everything it does is reversible by switching modes or deleting those files.

## Why the trim

Most of what old Prelude bundled is now handled by the rest of the network:

- **BCAT content** (Splatoon 2/3, SMB35 events) → served by `nextendo-bcat-nx`.
- **NEX/DataStore games** (MK8, SSBU, ACNH…) → just need the host redirect.
- **System services** (app-auth, version list, push) → `nextendo-aauth-nx`,
  `nextendo-tagaya-nx`, `nextendo-npns-nx`.

So the app keeps only the irreducible core — flip the redirect, manage cert trust,
stay reversible — and hands the content to the servers. See
[`RESTRUCTURE.md`](RESTRUCTURE.md) for the full trim rationale and
[`NOTES.md`](NOTES.md) for build status.

The addresses it redirects to live in `include/nextendo/config.hpp` and the host
list in `source/core/hosts.cpp` — that is the whole configuration surface.

## Building

Requires devkitPro + Aether. See [`NOTES.md`](NOTES.md). In short:

```sh
git submodule update --init --recursive
make -C lib/Aether
make -j$(nproc)          # -> nextendo-nx.nro
```

> **Status:** this is the Aether rewrite in progress; the C++ has been reviewed
> but not yet compiled on a real toolchain. `NOTES.md` lists the first-build
> checklist.

## Credits / sources

- **Aether** — [tallbl0nde/Aether](https://github.com/tallbl0nde/Aether), the GUI
  library TriPlayer is built on.
- **Atmosphère** — DNS.mitm, `system_settings.ini`, ExeFS/NRO patch format.
- Original **Prelude** host list and Atmosphère invariants, captured by the
  Nextendo Network contributors against production containers.

## Licence

Copyright (C) 2026 Nextendo Network. Licensed under the **PolyForm Shield License
1.0.0** — use, study, share and modify for any purpose except providing a product
that competes with Nextendo Network. See [LICENSE.md](LICENSE.md).

Not affiliated with, endorsed by, or connected to Nintendo. "Nintendo" and
"Nintendo Switch" are trademarks of Nintendo. Use it on hardware you own.
