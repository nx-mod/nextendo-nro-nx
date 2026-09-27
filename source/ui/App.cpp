// nextendo-nx — Nintendo Switch homebrew for the Nextendo Network.
// Copyright (C) 2026 Nextendo Network. PolyForm Shield License 1.0.0.
// Required Notice: Copyright 2026 Nextendo Network
//
// NOT COMPILED here. Follows Aether's Application/Screen/Overlay API; verify
// against the real headers (lib/Aether) at build time.
#include "ui/App.hpp"
#include "ui/PickerScreen.hpp"
#include "ui/SettingsScreen.hpp"
#include "ui/DiagScreen.hpp"
#include "nextendo/config.hpp"

namespace ui {

App::App() : Aether::Application(Aether::WindowMode::Normal) {
    // Follow the system HOME-menu / Settings theme (light or dark).
    this->setHighlightColours(Aether::Colour{255, 255, 255, 0},
                              Aether::Colour{255, 90, 20, 255}); // Nextendo orange accent
    this->setBackgroundColour(30, 32, 38); // Aether re-themes for light mode

    serverIp = nextendo::config::kServerDefault;
    mode     = nextendo::apply::currentMode();

    picker   = new PickerScreen(this);
    settings = new SettingsScreen(this);
    diag     = new DiagScreen(this);

    this->addScreen(picker);
    this->addScreen(settings);
    this->addScreen(diag);
    this->setScreen(picker);
}

App::~App() {
    // Aether::Application deletes added screens.
}

void App::showPicker()   { this->setScreen(picker); }
void App::showSettings() { this->setScreen(settings); }
void App::showDiag()     { this->setScreen(diag); }

void App::applyAndReboot(nextendo::apply::Mode m) {
    // A blocking overlay while we write files, then reboot (which does not
    // return). On failure we drop the overlay and surface a message box.
    auto *ovl = new Aether::Overlay();
    auto *msg = new Aether::Text(0, 0,
        m == nextendo::apply::Mode::Nextendo ? "Switching to Nextendo…"
                                             : "Switching to Nintendo…", 26);
    msg->setColour(this->theme()->text());
    ovl->addElement(msg);
    msg->setX(640 - msg->w() / 2);
    msg->setY(360 - msg->h() / 2);
    this->addOverlay(ovl);

    bool ok = (m == nextendo::apply::Mode::Nextendo)
                  ? nextendo::apply::applyNextendo(serverIp)
                  : nextendo::apply::applyNintendo();

    if (ok) {
        nextendo::apply::reboot(); // does not return on success
    }

    // If we get here, apply or reboot failed.
    this->deleteOverlay(ovl);
    this->createMessageBox("Failed",
        "Could not apply the change. See the trace on your SD card.", "OK");
}

} // namespace ui
