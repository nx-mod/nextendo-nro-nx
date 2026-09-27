# lib/

Third-party libraries live here as git submodules.

## Aether

The GUI library nextendo-nx is built on (the one TriPlayer uses). Add it:

```sh
git submodule add https://github.com/tallbl0nde/Aether lib/Aether
git submodule update --init --recursive
make -C lib/Aether        # builds lib/Aether/lib/libaether.a
```

It is intentionally NOT vendored here (no submodule committed) so this branch
carries only nextendo-nx's own code. See ../NOTES.md.
