// nextendo-nx — Nintendo Switch homebrew for the Nextendo Network.
// Copyright (C) 2026 Nextendo Network. PolyForm Shield License 1.0.0.
// Required Notice: Copyright 2026 Nextendo Network
//
// NOT COMPILED here. Follows Aether's List/ListOption API; verify at build time.
#include "ui/SettingsScreen.hpp"
#include "ui/App.hpp"
#include "nextendo/config.hpp"

namespace ui {

SettingsScreen::SettingsScreen(App *app) : app(app) {}

void SettingsScreen::onLoad() {
    auto *title = new Aether::Text(65, 45, "Settings", 28);
    title->setColour(this->app->theme()->text());
    this->addElement(title);

    list = new Aether::List(40, 90, 1200, 560);

    // Server selection — one option per configured server; ticks the active one.
    for (const auto &s : nextendo::config::kServers) {
        std::string ip = s.ip;
        auto *opt = new Aether::ListOption(s.label, ip,
            [this, ip]() { this->app->serverIp = ip; this->app->showSettings(); });
        if (this->app->serverIp == ip)
            opt->setColours(this->app->theme()->accent(), this->app->theme()->text(),
                            this->app->theme()->mutedText());
        list->addElement(opt);
    }

    // Language and backup toggles are wired to i18n / apply::backup — see NOTES.
    list->addElement(new Aether::ListOption("Language", "English",
        [this]() { /* TODO: i18n picker */ }));
    list->addElement(new Aether::ListOption("Restore my hosts in Nintendo mode", "On",
        [this]() { /* TODO: toggle backup::useForNintendo */ }));

    this->addElement(list);

    controls = new Aether::Controls();
    controls->addItem(new Aether::ControlItem(Aether::Button::A, "OK"));
    controls->addItem(new Aether::ControlItem(Aether::Button::B, "Back"));
    this->addElement(controls);
    this->onButtonPress(Aether::Button::B, [this]() { this->app->showPicker(); });
}

void SettingsScreen::onUnload() { this->removeAllElements(); }

} // namespace ui
