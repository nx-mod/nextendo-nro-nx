# nextendo-nx — Prelude rewrite plan (trim + Aether port)

> This document is the design for rewriting **Prelude** into **nextendo-nx**: a
> minimal Switch homebrew that flips the console between the Nextendo Network and
> Nintendo's servers. It records what the current app does, what the rest of the
> Nextendo Network now handles for us, what we drop, and the shape of the Aether
> (C++) rewrite.
>
> **Build status: builds.** `make -C lib/Aether && make` produces
> `nextendo-nx.nro` with `devkitpro/devkita64`, verified from a clean checkout.
> The remaining work is i18n and persisting the Settings toggles. See NOTES.md.

## 1. Why rewrite

The current Prelude is ~9,600 lines of C (plus a vendored 9,400-line miniz), a
custom framebuffer UI drawn by hand with FreeType, an mpg123 audio thread, a
self-updater, and **14 MB of bundled game content** in `romfs/`:

| romfs payload | size | what it is |
| --- | --- | --- |
| `ssbu_quickplay/` | 5.9 MB | full SSBU "Online Deluxe" mod stack (ARCropolis, skyline, OC) |
| `bgm.mp3` | 4.7 MB | background music |
| `bcatdata/` | 1.8 MB | Splatoon 2/3 BCAT festival/schedule containers |
| `sd/` | 556 KB | ExeFS/NRO IPS patch sets, CA bundle |
| `smb35_spbattle/` | 56 KB | SMB35 Special Battle exefs replacement |
| fonts + logos | ~0.8 MB | Poppins family, RGBA splashes |

The app has grown into a **content-delivery bundle with a settings UI bolted on**.
Every weekly release re-ships megabytes of mods and patches that go stale against
game/Atmosphère updates. The user's call: *"its embedded modules and code is
overkill and we can do things better now. Prelude should be trimmed to as little
as possible — host redirect should handle most of it."*

That is correct, and the network has caught up to make it true:

- **`nextendo-bcat-nx`** now serves BCAT/d4c containers signed with our own key.
  Splatoon 2/3 and SMB35 event data no longer need to ship inside the `.nro` —
  the console pulls it from the BCAT server once DNS.mitm points there.
- **Host redirect + DNS.mitm** already carries every NEX/DataStore game (MK8,
  SSBU, ACNH, Strikers, LM3, ARMS, Tennis) with no per-game files at all.
- **`nextendo-aauth-nx` / `nextendo-tagaya-nx` / `nextendo-npns-nx`** answer the
  system services (app-auth token, version list, push) that the redirect points
  at, so the console trusts the stack without local sysmodules.
- Certificate trust is the one thing that genuinely must live on the SD (ExeFS
  patches are matched by build-id at boot). That stays — but as a **small,
  versioned patch set**, not bundled per game with mod stacks attached.

So nextendo-nx keeps the irreducible core and hands everything else to the
servers.

## 2. What the rewrite keeps, moves, and drops

| Current feature | Decision | Rationale |
| --- | --- | --- |
| Mode picker (NEXTENDO / NINTENDO) | **keep** | the whole point of the app |
| Write `hosts/{sysmmc,emummc}.txt` | **keep** | irreducible core |
| Toggle `enable_dns_mitm` in `system_settings.ini` | **keep** | irreducible core |
| PRODINFO per-mode (`exosphere.ini`, emuMMC) | **keep** | reversibility contract |
| Reboot (bpc) | **keep** | apply needs it |
| Server IP selection (default / alt / nncs2) | **keep, simplify** | still needed; move to a config file, not a hidden ↑↓←→ code |
| Hosts backup before overwrite | **keep** | user-data safety contract |
| Certificate trust patch set (`exefs_patches`, CA bundle) | **keep, slim** | must be on SD, matched by build-id; ship only cert/peer patches |
| Telemetry blocking (both modes) | **keep** | one INI/hosts stanza, cheap |
| Self-updater (GitHub releases) | **keep, simplify** | useful, but drop the "stale bundled mod" detection since mods are gone |
| **SSBU quickplay mod (5.9 MB)** | **DROP from bundle** | distribute as its own release/asset the updater can fetch on demand; not core |
| **`bgm.mp3` (4.7 MB) + mpg123 + audio.c** | **DROP** | Aether has no need for it; removes a dependency and a thread |
| **Splatoon 2/3 BCAT data (1.8 MB)** | **DROP from bundle** | served by `nextendo-bcat-nx` now |
| **SMB35 Special Battle (56 KB) + installer** | **DROP from bundle** | optional content → BCAT server / separate asset |
| **Country flag installer (110 flags, `nextendo_flag.c`)** | **MOVE** | build-on-device patch belongs in an optional tool, not the core flow |
| **Account-link fallback sysmodule** | **KEEP as optional patch**, no UI wizard | ship the sysmodule dir under Nextendo mode; drop the bespoke warn screen |
| **S3 patch-status screen** | **SIMPLIFY** | one line in a "diagnostics" view, not its own screen family |
| Custom framebuffer UI (`ui.c`, `ui_theme.h`) | **REPLACE** | Aether |
| `lang.c` (55 KB hand-rolled i18n, 5 languages) | **REPLACE** | Aether-friendly string table, keep the 5 languages |
| `miniz` (vendored zip) | **KEEP only if** the on-demand mod fetch needs it | otherwise drop |

Net effect: the shipped `.nro` goes from ~14 MB of romfs to **fonts + a small
cert-patch set + logo** (well under 1 MB), and the source from ~10k lines of C to
a few thousand lines of C++ with Aether owning the UI.

## 3. Target structure

```
nextendo-nx/
├─ Makefile                 # devkitA64, links Aether + libnx (no mpg123/freetype-direct)
├─ README.md
├─ NOTES.md                 # honest build state + deferred items
├─ RESTRUCTURE.md           # this file
├─ icon.jpg
├─ lib/
│  └─ Aether/               # submodule: nx-mod/Aether (tallbl0nde/Aether)
├─ include/
│  └─ nextendo/
│     ├─ config.hpp         # server list, paths, build id
│     ├─ hosts.hpp          # DNS.mitm hosts builder
│     ├─ apply.hpp          # write hosts / edit ini / prodinfo / reboot
│     ├─ backup.hpp         # user hosts backup/restore
│     ├─ update.hpp         # GitHub self-update
│     └─ i18n.hpp           # string table
├─ source/
│  ├─ main.cpp              # Aether::Application bootstrap
│  ├─ core/
│  │  ├─ hosts.cpp          # ported from nextendo_hosts.h/apply.c (VERIFIED-BY-REVIEW)
│  │  ├─ apply.cpp          # ported system logic
│  │  ├─ backup.cpp
│  │  ├─ update.cpp
│  │  └─ i18n.cpp
│  └─ ui/
│     ├─ App.cpp            # Application subclass, screen wiring
│     ├─ PickerScreen.cpp   # NEXTENDO / NINTENDO chooser (the home screen)
│     ├─ ApplyOverlay.cpp   # progress + reboot
│     ├─ SettingsScreen.cpp # server IP, language, backup toggle
│     └─ DiagScreen.cpp     # current mode, patch status, network test
└─ romfs/
   ├─ fonts/                # Poppins (Aether can also use its bundled font)
   ├─ logo.png
   └─ sd/atmosphere/        # ONLY cert-trust patches + account-link sysmodule
```

Screen map (was 15 `SCREEN_*` states in one 769-line `main()` switch):

```
PickerScreen ──select NEXTENDO──▶ ApplyOverlay(nextendo) ──▶ reboot
             ──select NINTENDO──▶ ApplyOverlay(nintendo) ──▶ reboot
             ──(+)──▶ SettingsScreen (server, language, backup)
             ──(-)──▶ DiagScreen (mode, patches, net test)
             ──launch, update available──▶ UpdateOverlay
```

Everything BCAT/mod/flag/SMB35/S2/S3-wizard related is gone from the flow; the
picker + settings + diagnostics is the entire app.

## 4. Core logic port (the part that must stay correct)

The irreducible core is `hosts_build()` + `apply_nextendo()/apply_nintendo()`.
These are pure file/string logic and port almost 1:1 from C. They are
reimplemented in `source/core/hosts.cpp` and `apply.cpp` as reviewed ports, with
the Atmosphère invariants preserved verbatim:

- **"last matching line wins"** ordering in the hosts file (MK8 nncs2 override
  must stay *after* the `g2*` wildcard).
- Two **distinct** IPs for nncs1/nncs2 (Pia dedups identical probes → NAT never
  completes → MK8/S2 error 2618-201).
- `enable_dns_mitm = u8!0x1`, `add_defaults_to_dns_hosts = u8!0x0`.
- `fsdevCommitDevice("sdmc")` before reboot or writes are lost on POWER.
- Write BOTH `sysmmc.txt` and `emummc.txt` regardless of detected boot storage.
- Nintendo mode restores telemetry-blocking defaults (merged, not replaced) and
  purges logs that could leak the server IP.

The host list itself is unchanged from the audited production list in
`nextendo_hosts.h` (host ids were captured off production containers, not
guessed) — the rewrite keeps the same literal.

## 5. Deferred / needs a real toolchain

- Compile against real Aether + libnx headers; the UI classes here follow
  Aether's `Application`/`Screen`/`Element` API but are **unverified**.
- Wire the on-demand SSBU-mod fetch to a GitHub release asset (replaces the 5.9
  MB bundle) — needs the updater's zip path (`miniz`) kept if we go this route.
- Confirm Aether's font/theme hooks reproduce the "follow system light/dark
  theme" behaviour the old UI had.
- Port the 5-language string table out of `lang.c` into `i18n.cpp` (mechanical).

## Credits / sources

- **Aether** — tallbl0nde/Aether (the GUI library TriPlayer uses), mirrored at
  nx-mod/Aether.
- **Atmosphère** — DNS.mitm, `system_settings.ini`, ExeFS/NRO patch format.
- Original Prelude host list and Atmosphère invariants captured by the Nextendo
  Network contributors against production containers.
