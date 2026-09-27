# nextendo-nx — build & status notes

## Honest state

This branch is the **Aether rewrite scaffold** of Prelude, produced as a
restructure + reviewed port. It has **not been compiled**: the environment it was
authored in has no devkitPro / libnx / Aether toolchain. Treat every `.cpp/.hpp`
as logic-reviewed, not build-verified.

What is real and reviewable:

- **`source/core/hosts.cpp`** — the DNS.mitm hosts builder. Every functional host
  line is verbatim from the audited production list (`nextendo_hosts.h` /
  `nextendo_apply.c` on `main`); only `#` comments were translated to English.
- **`source/core/apply.cpp`** — write hosts, toggle `enable_dns_mitm` /
  `add_defaults_to_dns_hosts`, per-mode `blank_prodinfo_emummc`, provision the
  cert-trust patch set, back up the user's `default.txt`, purge leak-y logs,
  reboot. The INI-editing semantics match the original `iniSetDnsMitm` /
  `iniSetBlankProdinfoEmummc` (section-aware, replaces commented keys in place).

What is a scaffold following Aether's API but **unverified against real headers**:

- `source/main.cpp`, `source/ui/*` — `Application` / `Screen` / `Element` usage.
  Method names (`createMessageBox`, `theme()->text()`, `FilledButton`, `List`,
  `Controls`, `Overlay`, `TextBlock`) follow Aether's conventions as used by
  TriPlayer, but must be checked against `lib/Aether/include` when building.

## To build (on a real toolchain)

```sh
# 1. Vendor Aether (the GUI library TriPlayer uses).
git submodule add https://github.com/nx-mod/Aether lib/Aether   # or tallbl0nde/Aether
git submodule update --init --recursive
make -C lib/Aether                 # produces lib/Aether/lib/libaether.a

# 2. Build the app.
export DEVKITPRO=/opt/devkitpro
dkp-pacman -Syu --noconfirm switch-sdl2 switch-sdl2_ttf switch-sdl2_gfx \
                            switch-sdl2_image switch-freetype
make -j$(nproc)                    # -> nextendo-nx.nro
```

Or, matching the old Docker one-liner (adjust packages for the SDL2 stack):

```sh
docker run --rm -v "$PWD:/work" -w /work devkitpro/devkita64 \
  bash -c 'dkp-pacman -Syu --noconfirm switch-sdl2 switch-sdl2_ttf \
           switch-sdl2_gfx switch-sdl2_image switch-freetype && \
           make -C lib/Aether && make -j$(nproc)'
```

## First-build checklist (expected fixes)

1. Reconcile Aether method names with `lib/Aether/include` (see above).
2. Confirm the SDL2 link line — package names / lib order may differ from the
   list in the `Makefile`.
3. Port the 5-language string table out of the old `lang.c` into
   `include/nextendo/i18n.hpp` + `source/core/i18n.cpp` (currently English-only
   literals in the UI). Mechanical.
4. Decide the on-demand SSBU-mod fetch (was a 5.9 MB bundle): either drop it
   entirely or have the updater pull a GitHub release asset. If kept, restore a
   zip lib (miniz) for it.

## Trimmed vs. old Prelude

Dropped from the `.nro`: `ssbu_quickplay/` (5.9 MB), `bgm.mp3` (4.7 MB) + mpg123
audio thread, `bcatdata/` (1.8 MB, now served by `nextendo-bcat-nx`),
`smb35_spbattle/`, the flag installer, the custom framebuffer UI, and the 15-state
`main()` switch. Kept: mode picker, hosts/INI/PRODINFO apply, cert-trust patch set
(`romfs/sd`), backup, self-update, telemetry blocking. See `RESTRUCTURE.md`.
