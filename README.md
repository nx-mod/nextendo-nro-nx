# nextendo-prelude (nx-mod testing)

nx-mod's `testing` fork of [Prelude](https://github.com/NextendoNetwork/Prelude-Nro), the Nextendo
Network homebrew for a real Nintendo Switch. Part of
[nextendo-testing](https://github.com/nx-mod/nextendo-testing): the whole network, run on a LAN.
Upstream's README is kept as [README.upstream.md](README.upstream.md).

## nx-mod changes

- **LAN build.** `make LAN_HOST=<ip> [LAN_HOST2=<ip>]` makes the LAN stack the default server
  (real Nextendo stays the alternative). Without `LAN_HOST` the build is upstream's.
- **Local CA in the browser bundles.** The console's browser trusts `Nextendo Local CA`, the
  stack's CA, next to Nextendo's own; the account-link page loads from the LAN stack.
- **No mandatory update lock** (`PRELUDE_MANDATORY_UPDATE=0`): no GitHub check at start.
- **Hosts:** `*.demonware.net` (Diablo III), and the five specific srv hosts written below the
  wildcards.

## Build

devkitPro (devkitA64) and `make`, or `make LAN_HOST=<your stack's address>`. In nextendo-testing,
`build_all.ps1` builds it for the address in `stack.cfg`.

## Credits

Prelude is the work of the **Nextendo Network team** — https://nextendo.network. nx-mod only adds
the changes above for LAN testing. Nextendo is awesome.
