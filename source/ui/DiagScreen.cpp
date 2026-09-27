// nextendo-nx — Nintendo Switch homebrew for the Nextendo Network.
// Copyright (C) 2026 Nextendo Network. PolyForm Shield License 1.0.0.
// Required Notice: Copyright 2026 Nextendo Network
//
// NOT COMPILED here. Follows Aether's TextBlock API; verify at build time.
#include "ui/DiagScreen.hpp"
#include "ui/App.hpp"
#include "nextendo/apply.hpp"

namespace ui {

using nextendo::apply::Mode;
using nextendo::apply::Boot;

DiagScreen::DiagScreen(App *app) : app(app) {}

void DiagScreen::onLoad() {
    auto *title = new Aether::Text(65, 45, "Diagnostics", 28);
    title->setColour(this->app->theme()->text());
    this->addElement(title);

    Mode m = nextendo::apply::currentMode();
    Boot b = nextendo::apply::detectBoot();
    const char *bootStr = b == Boot::Emummc ? "emuMMC"
                        : b == Boot::Sysmmc ? "sysNAND" : "unknown";

    std::string text;
    text += "Loaded mode:   ";
    text += (m == Mode::Nextendo) ? "NEXTENDO\n" : "NINTENDO\n";
    text += "Boot storage:  "; text += bootStr; text += "\n";
    text += "Server:        "; text += this->app->serverIp; text += "\n";
    text += "\n";
    text += "In Nextendo mode, Nintendo traffic is redirected to the server\n";
    text += "above via Atmosphere DNS.mitm. Splatoon/SMB35 BCAT content is\n";
    text += "served by nextendo-bcat-nx, not bundled in this app.\n";

    body = new Aether::TextBlock(65, 100, text, 22, 1150);
    body->setColour(this->app->theme()->text());
    this->addElement(body);

    controls = new Aether::Controls();
    controls->addItem(new Aether::ControlItem(Aether::Button::B, "Back"));
    this->addElement(controls);
    this->onButtonPress(Aether::Button::B, [this]() { this->app->showPicker(); });
}

void DiagScreen::onUnload() { this->removeAllElements(); }

} // namespace ui
