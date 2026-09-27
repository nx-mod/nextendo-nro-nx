// nextendo-nx — Nintendo Switch homebrew for the Nextendo Network.
// Copyright (C) 2026 Nextendo Network. PolyForm Shield License 1.0.0.
// Required Notice: Copyright 2026 Nextendo Network
#pragma once
#include <Aether/Aether.hpp>

namespace ui {
class App;

// Diagnostics: current mode, boot storage, whether DNS.mitm is on and our hosts
// carry the current IP, and a basic reachability line. Folds the old S3
// patch-status screen family into one read-only view.
class DiagScreen : public Aether::Screen {
public:
    explicit DiagScreen(App *app);
    void onLoad() override;
    void onUnload() override;
private:
    App *app;
    Aether::TextBlock *body = nullptr;
    Aether::Controls  *controls = nullptr;
};

} // namespace ui
