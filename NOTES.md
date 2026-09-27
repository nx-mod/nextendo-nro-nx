# nextendo-nx — build & status notes

## State

**Builds.** `make -C lib/Aether && make` produces `nextendo-nx.nro` with the
`devkitpro/devkita64` toolchain (the SDL2 stack + freetype + harfbuzz it needs
ship in that image). Verified from a clean checkout this session.

- **`source/core/{hosts,apply}.cpp`** — the DNS.mitm hosts builder and the system
  logic (write hosts, toggle `enable_dns_mitm` / `add_defaults_to_dns_hosts`,
  per-mode `blank_prodinfo_emummc`, provision the cert-trust patch set, back up
  the user's `default.txt`, reboot). Host lines are verbatim from the audited
  production list; INI-edit semantics match the original.
- **`source/main.cpp` + `source/ui/*`** — the Aether UI: a HOME-menu-style
  `Menu` (Networks / Settings / Diagnostics / About) with a content pane that
  swaps per selection. Compiles and links against real Aether.
- **`source/ui/probe.cpp`** — the LAN server-reachability check on the
  Diagnostics pane (TCP connect + select).

## Building

```sh
git clone https://github.com/nx-mod/nextendo-nx    # (repo will be renamed)
cd nextendo-nx
git submodule update --init --recursive            # fetches lib/Aether

# with Docker (no local devkitPro needed):
docker run --rm -e HOME=/tmp -v "$PWD:/work" -w /work devkitpro/devkita64 \
  bash -c 'make -C lib/Aether -j$(nproc) && make -j$(nproc)'
# -> nextendo-nx.nro   (copy to sd:/switch/)
```

`lib/Aether` is a git submodule (tallbl0nde/Aether); `make -C lib/Aether` builds
`libAether.a` once, then the app links it.

## Still open / nice-to-have

- **i18n**: the UI strings are English literals. The old `lang.c` carried
  English / Español / Português / Français / 中文; port that table into a small
  `i18n` module and wire the Settings "Language" option (currently a stub).
- **Settings toggles**: "Language" and "Restore my hosts in Nintendo mode" are
  stubbed (no persistence yet). Back them with a small config file at
  `sd:/switch/nextendo-nx/config.ini`.
- **On-demand mod fetch**: the old SSBU bundle (5.9 MB) is gone; if it's wanted,
  have the (not-yet-added) self-updater pull it from a GitHub release asset.
- **Self-updater**: not yet ported from old Prelude; add if desired.
- On-console test: it builds, but has not been run on hardware/emulator here.

## Trimmed vs. old Prelude

Dropped from the `.nro`: `ssbu_quickplay/` (5.9 MB), `bgm.mp3` (4.7 MB) + audio,
`bcatdata/` (1.8 MB, now served by `nextendo-bcat-nx`), `smb35_spbattle/`, the
flag installer, the custom framebuffer UI, and the 15-state `main()` switch.
Kept: mode picker, hosts/INI/PRODINFO apply, cert-trust patch set (`romfs/sd`),
telemetry blocking. UI source is ~960 lines (was ~10k lines of C). See
`RESTRUCTURE.md`.
