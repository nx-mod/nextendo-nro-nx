# nextendo-nx (nx-mod testing)

**A rewrite of [Prelude](https://github.com/NextendoNetwork/Prelude-Nro) by nx-mod.** The Nextendo Network
homebrew for a real Nintendo Switch, rebuilt from the ground up on the [Aether](https://github.com/tallbl0nde/Aether)
GUI and around nx-mod's new servers: BCAT, title versions (tagaya), push and the rest come from the network,
so the old bundled per-game data and workarounds are gone. Part of
[nextendo-testing](https://github.com/nx-mod/nextendo-testing): the whole Nextendo Network, run on a LAN.

It flips the console between the Nextendo Network and Nintendo's servers: DNS.mitm hosts, certificate
trust, per-mode PRODINFO, all reversible. Builds `nextendo-nx.nro`, which sits next to the stock Prelude.
Design notes: [RESTRUCTURE.md](RESTRUCTURE.md), [NOTES.md](NOTES.md), [README.nextendo-nx.md](README.nextendo-nx.md).

## LAN changes (testing)

- `make LAN_HOST=<ip> [LAN_HOST2=<ip>]` points every address at a LAN stack: the default server, nncs2
  (Pia needs a second address) and MK8. The production VPS stays the alternative.
- The browser bundles also trust `Nextendo Local CA`, the stack's CA, so the account-link page loads from it.

## Build

```sh
git submodule update --init --recursive   # lib/Aether
make -C lib/Aether && make LAN_HOST=192.168.137.1 LAN_HOST2=192.168.137.2
```

In nextendo-testing, `build_all.ps1` does this for the addresses in `stack.cfg`.

## Credits

Rewritten by **nx-mod**, on Prelude by the **Nextendo Network team** — https://nextendo.network.
Aether by tallbl0nde. Nextendo is awesome.
