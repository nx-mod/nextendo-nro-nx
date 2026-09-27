// nextendo-nx — Nintendo Switch homebrew for the Nextendo Network.
// Copyright (C) 2026 Nextendo Network. PolyForm Shield License 1.0.0.
// Required Notice: Copyright 2026 Nextendo Network
//
// NOT COMPILED here. Follows Aether's Screen/Element API; verify at build time.
#include "ui/PickerScreen.hpp"
#include "ui/App.hpp"
#include "nextendo/apply.hpp"

namespace ui {

using nextendo::apply::Mode;

PickerScreen::PickerScreen(App *app) : app(app) {}

void PickerScreen::onLoad() {
    // Rebuild each load so the status line reflects the current mode.
    heading = new Aether::Text(0, 130, "Which network do you want to load?", 30);
    heading->setColour(this->app->theme()->text());
    heading->setX(640 - heading->w() / 2);
    this->addElement(heading);

    // NEXTENDO — primary action.
    nextendo = new Aether::FilledButton(340, 260, 260, 150, "NEXTENDO",
        34, [this]() {
            this->app->createMessageBox(
                "Switch to Nextendo?",
                "Redirects Nintendo traffic to the Nextendo Network and reboots.",
                "Switch", "Cancel",
                [this]() { this->app->applyAndReboot(Mode::Nextendo); });
        });
    nextendo->setFillColour(Aether::Colour{255, 90, 20, 255});
    this->addElement(nextendo);

    // NINTENDO — restore official servers.
    nintendo = new Aether::BorderButton(680, 260, 260, 150, 3, "NINTENDO",
        34, [this]() {
            this->app->createMessageBox(
                "Switch to Nintendo?",
                "Removes everything Nextendo installed and restores official servers, then reboots.",
                "Switch", "Cancel",
                [this]() { this->app->applyAndReboot(Mode::Nintendo); });
        });
    nintendo->setBorderColour(this->app->theme()->mutedText());
    this->addElement(nintendo);

    // Status line: which mode is loaded right now.
    Mode m = nextendo::apply::currentMode();
    const char *label = (m == Mode::Nextendo) ? "Currently loaded: NEXTENDO"
                                              : "Currently loaded: NINTENDO";
    statusLine = new Aether::Text(0, 470, label, 22);
    statusLine->setColour(this->app->theme()->mutedText());
    statusLine->setX(640 - statusLine->w() / 2);
    this->addElement(statusLine);

    // Focus the button for the OTHER mode (the likely next action).
    this->setFocused(m == Mode::Nextendo ? (Aether::Element *)nintendo
                                         : (Aether::Element *)nextendo);

    // Button hints.
    controls = new Aether::Controls();
    controls->addItem(new Aether::ControlItem(Aether::Button::A,    "Select"));
    controls->addItem(new Aether::ControlItem(Aether::Button::PLUS, "Settings"));
    controls->addItem(new Aether::ControlItem(Aether::Button::MINUS,"Diagnostics"));
    controls->addItem(new Aether::ControlItem(Aether::Button::B,    "Exit"));
    this->addElement(controls);

    this->onButtonPress(Aether::Button::PLUS,  [this]() { this->app->showSettings(); });
    this->onButtonPress(Aether::Button::MINUS, [this]() { this->app->showDiag(); });
    this->onButtonPress(Aether::Button::B,     [this]() { this->app->exit(); });
}

void PickerScreen::onUnload() {
    this->removeAllElements();
}

} // namespace ui
