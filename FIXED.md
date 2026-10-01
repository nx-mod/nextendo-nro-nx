# Fixed — nextendo-nro-nx

- **No way to see or add console news**: Diagnostics → Dump / Post / Clear news (local news, no network).
- **No news after Clear news**: clearing dropped the channel subscriptions; Clear news now subscribes the default
  channels again, and **Subscribe** (a diagnostic) subscribes them and asks for them now.
- **Message boxes**: a separator line between the text and the buttons.
- **Crash on closing a message box** (e.g. Clear news): a closed box was freed while the Window and its button's
  callback still used it; it is now freed a frame later, and actions that swap a box or rebuild a pane run after
  the button press is handled (they crashed on Clear news too).
- **BCAT rejected Nextendo content**: `nextendo_bcat_sig` patch staged in romfs (bcat 22.5.0), covering all four
  signature checks (PSS and PKCS#1 v1.5, SHA-1 and SHA-256), not only PSS SHA-256.
- **Half-linked users could not be deleted (2002-0001)**: Users → Unlink removes the link on this console only, then
  offers a reboot.

## Credits

- [sys-patch](https://github.com/impeeza/sys-patch) — the exefs patch approach.
- [Linkalho](https://github.com/impeeza/linkalho) (original by rdmrocha) — the local unlink approach, reimplemented.
- The whole Nextendo Network team — https://nextendo.network. Nextendo is awesome.
