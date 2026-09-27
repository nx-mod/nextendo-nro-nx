// nextendo-nx — Nintendo Switch homebrew for the Nextendo Network.
// Copyright (C) 2026 Nextendo Network. PolyForm Shield License 1.0.0.
// Required Notice: Copyright 2026 Nextendo Network
#pragma once
#include <Aether/Aether.hpp>

namespace ui {
class App;

// Settings: choose the server (production / alternate), language, and whether to
// restore the user's own hosts on switching back to Nintendo. Replaces the old
// hidden ↑↓←→ server-swap code and scattered toggles.
class SettingsScreen : public Aether::Screen {
public:
    explicit SettingsScreen(App *app);
    void onLoad() override;
    void onUnload() override;
private:
    App *app;
    Aether::List     *list = nullptr;
    Aether::Controls *controls = nullptr;
};

} // namespace ui
