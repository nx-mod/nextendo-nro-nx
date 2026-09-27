// nextendo-nx — Nintendo Switch homebrew for the Nextendo Network.
// Copyright (C) 2026 Nextendo Network. PolyForm Shield License 1.0.0.
// Required Notice: Copyright 2026 Nextendo Network
//
// Entry point. Aether owns the render loop; MainScreen wires the menu + panes.
// All system work (hosts, DNS.mitm, PRODINFO, reboot) lives in nextendo::apply.
#include <Aether/Aether.hpp>
#include <switch.h>

#include "ui/MainScreen.hpp"
#include "ui/theme.hpp"

int main(int, char **) {
    // Sockets for the LAN server-reachability probe on the Diagnostics pane.
    socketInitializeDefault();
    romfsInit();

    auto *window = new Aether::Window(
        "Nextendo", 1280, 720,
        [](const std::string, const bool) {}); // swallow Aether's log

    window->setBackgroundColour(ui::theme::Background);
    window->setHighlightBackground(ui::theme::HighlightBg);
    window->setHighlightOverlay(Aether::Colour(255, 255, 255, 40));
    // Nextendo-orange highlight border, gently pulsing.
    window->setHighlightAnimation([](const uint32_t t) -> Aether::Colour {
        const double f = 0.5 + 0.5 * std::sin(t / 400.0);
        return Aether::Colour(255, 120 + (int)(30 * f), 30, 255);
    });
    window->setFont("romfs:/fonts/Poppins-Regular.ttf");

    auto *screen = new ui::MainScreen(window);
    window->showScreen(screen);

    while (window->loop())
        ;

    delete screen;
    delete window;
    romfsExit();
    socketExit();
    return 0;
}
