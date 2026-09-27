# Running nextendo-nx (prebuilt .nro)

`nextendo-nx.nro` is the homebrew that switches your console between the Nextendo
Network and Nintendo. No build needed.

1. Copy `nextendo-nx.nro` to your SD card under `/switch/`.
2. On the console, open the homebrew menu (album) and launch **Nextendo**.
3. Pick **NEXTENDO** to go online on the Nextendo servers, or **NINTENDO** to
   restore the official servers. It applies the change and reboots.

The **Diagnostics** screen tests whether your Nextendo servers are reachable on
the LAN. The server it points at is set in **Settings** (default is the built-in
production IP; change it there for your own box).

> Built with devkitpro/devkita64. Requires Atmosphère CFW.
