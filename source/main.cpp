// nextendo-nx — Nintendo Switch homebrew for the Nextendo Network.
// Copyright (C) 2026 Nextendo Network. PolyForm Shield License 1.0.0.
// Required Notice: Copyright 2026 Nextendo Network
//
// Entry point. Aether owns the render loop; App wires the screens. All the
// system work (writing hosts, toggling DNS.mitm, reboot) lives in
// nextendo::apply — see include/nextendo/apply.hpp.
//
// NOT COMPILED in the authoring environment (no devkitPro/Aether). See NOTES.md.
#include "ui/App.hpp"

int main(int, char **) {
    ui::App app;
    app.run();   // blocks until the user exits or the console reboots
    return 0;
}
