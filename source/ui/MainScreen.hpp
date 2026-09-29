// nextendo-nx — main screen (menu + panes).
// Copyright (C) 2026 Nextendo Network. PolyForm Shield License 1.0.0.
// Required Notice: Copyright 2026 Nextendo Network
#pragma once
#include <Aether/Aether.hpp>
#include <functional>
#include <string>
#include <utility>
#include <vector>
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
    void update(unsigned int dt) override;

private:
    Aether::Window *window;
    Aether::Container *content = nullptr; // right-hand pane, rebuilt per menu item
    Aether::MessageBox *msg = nullptr;    // current modal (owned, deleted on replace)
    // Closed modals and the frames since: the Window still holds a closed overlay until its next update (and a
    // button's callback is still running in it), so they are deleted a frame later.
    std::vector<std::pair<Aether::MessageBox *, int>> closing;
    // Actions queued from input callbacks, run in update(): replacing a modal or rebuilding a pane while Aether
    // is still dispatching the button press called into freed callbacks (crashes on Clear news).
    std::vector<std::function<void()>> pending;
    void later(std::function<void()> fn) { pending.push_back(std::move(fn)); }
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
    void showInfo(const std::string &text); // an OK message box with centred text (opened next frame)
    void openInfo(const std::string &text);
};

} // namespace ui
