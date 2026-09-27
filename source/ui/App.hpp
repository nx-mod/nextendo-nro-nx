// nextendo-nx — Nintendo Switch homebrew for the Nextendo Network.
// Copyright (C) 2026 Nextendo Network. PolyForm Shield License 1.0.0.
// Required Notice: Copyright 2026 Nextendo Network
#pragma once
#include <Aether/Aether.hpp>
#include <string>
#include "nextendo/apply.hpp"

// Application shell. Owns the screens and the small shared state the UI needs:
// the currently-loaded mode and the chosen server IP. Aether drives the loop
// (App::run) and light/dark theming follows the system.
namespace ui {

class PickerScreen;
class SettingsScreen;
class DiagScreen;

class App : public Aether::Application {
public:
    App();
    ~App();

    // Shared state.
    std::string serverIp;                 // IP the apply step will write
    nextendo::apply::Mode mode;           // last-detected loaded mode

    // Screen navigation helpers.
    void showPicker();
    void showSettings();
    void showDiag();

    // Kick off an apply+reboot for the given mode, behind a progress overlay.
    void applyAndReboot(nextendo::apply::Mode m);

private:
    PickerScreen   *picker   = nullptr;
    SettingsScreen *settings = nullptr;
    DiagScreen     *diag     = nullptr;
};

} // namespace ui
