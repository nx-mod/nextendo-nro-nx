// nextendo-nx — Nintendo Switch homebrew for the Nextendo Network.
// Copyright (C) 2026 Nextendo Network. PolyForm Shield License 1.0.0.
// Required Notice: Copyright 2026 Nextendo Network
#pragma once
#include <Aether/Aether.hpp>

namespace ui {
class App;

// The home screen: "Which network do you want to load?" with two large buttons,
// NEXTENDO and NINTENDO. (+) opens Settings, (-) opens Diagnostics.
class PickerScreen : public Aether::Screen {
public:
    explicit PickerScreen(App *app);
    void onLoad() override;
    void onUnload() override;

private:
    App *app;
    Aether::Text          *heading  = nullptr;
    Aether::FilledButton  *nextendo = nullptr;
    Aether::BorderButton  *nintendo = nullptr;
    Aether::Text          *statusLine = nullptr;
    Aether::Controls      *controls = nullptr;
};

} // namespace ui
