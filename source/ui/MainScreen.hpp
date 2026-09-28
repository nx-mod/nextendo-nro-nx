// nextendo-nx — main screen (menu + panes).
// Copyright (C) 2026 Nextendo Network. PolyForm Shield License 1.0.0.
// Required Notice: Copyright 2026 Nextendo Network
#pragma once
#include <Aether/Aether.hpp>
#include <string>
#include "nextendo/apply.hpp"

namespace ui {

// The whole app on one screen: a left Menu (Networks / Settings / Diagnostics /
// About) and a content area on the right that swaps per selection. This is the
// nice, HOME-menu-style arrangement the old 15-state framebuffer switch is
// replaced with.
class MainScreen : public Aether::Screen {
public:
    explicit MainScreen(Aether::Window *window);

    void onLoad() override;
    void onUnload() override;

private:
    Aether::Window *window;
    Aether::Container *content = nullptr; // right-hand pane, rebuilt per menu item
    Aether::MessageBox *msg = nullptr;    // current modal (owned, deleted on replace)
    Aether::Menu *menu = nullptr;
    Aether::MenuOption *optNet = nullptr, *optSet = nullptr, *optUsers = nullptr, *optDiag = nullptr, *optAbout = nullptr;

    std::string serverIp;

    // Panes.
    void showNetworks();
    void showSettings();
    void showUsers();
    void showDiagnostics();
    void showAbout();

    // Helpers.
    void clearContent();
    void confirmSwitch(nextendo::apply::Mode mode);
    void doSwitch(nextendo::apply::Mode mode);
    void closeMsg();
    void showInfo(const std::string &text); // an OK message box with centred text
};

} // namespace ui
