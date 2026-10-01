# TODO — nextendo-nro-nx

## In progress

- **Game data** (Diagnostics): working; lists each installed game's BCAT settings to `sd:/switch/nextendo-nx/bcat/`.
  The garbled-name display is fixed (prints a name only when it is printable ASCII).
- **Users → Unlink**: test on the half-linked users (unlink, reboot, delete in System Settings).

## Left

- **`nextendo_bcat_sig`** (romfs `sd/atmosphere/exefs_patches/nextendo_bcat_sig/`): install it with Nextendo mode and
  remove it with Nintendo mode, like the other patch sets. Firmware-specific (bcat 22.5.0).
- **Remove `romfs/account_link/`**: not referenced by the code.

## Credits

- [sys-patch](https://github.com/impeeza/sys-patch) — the ctest/exefs patch approach.
- [Linkalho](https://github.com/impeeza/linkalho) (original by rdmrocha) — the local unlink approach, reimplemented.
- The whole Nextendo Network team — https://nextendo.network. Nextendo is awesome.
