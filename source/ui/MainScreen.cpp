// nextendo-nx — main screen (menu + panes).
// Copyright (C) 2026 Nextendo Network. PolyForm Shield License 1.0.0.
// Required Notice: Copyright 2026 Nextendo Network
#include "ui/MainScreen.hpp"
#include "ui/theme.hpp"
#include "ui/probe.hpp"
#include "nextendo/config.hpp"
#include "nextendo/hosts.hpp"

#include <switch.h>

namespace ui {

namespace cfg = nextendo::config;
using nextendo::apply::Mode;
using nextendo::apply::Boot;

// Layout constants.
static constexpr int kHeaderH = 88;
static constexpr int kMenuX = 40, kMenuW = 340;
static constexpr int kContentX = 420, kContentW = 820;

MainScreen::MainScreen(Aether::Window *window) : window(window) {
    serverIp = cfg::kServerDefault;
}

void MainScreen::onLoad() {
    // Header bar.
    auto *bar = new Aether::Rectangle(0, 0, 1280, kHeaderH);
    bar->setColour(theme::Panel);
    this->addElement(bar);
    auto *title = new Aether::Text(kMenuX, 22, "Nextendo", 40);
    title->setColour(theme::Accent);
    this->addElement(title);
    auto *sub = new Aether::Text(kMenuX + title->w() + 16, 38, "Network switcher", 22);
    sub->setColour(theme::Muted);
    this->addElement(sub);

    // Left menu.
    menu = new Aether::Menu(kMenuX, kHeaderH + 20, kMenuW, 720 - kHeaderH - 100);
    optNet = new Aether::MenuOption("Networks", theme::Accent, theme::Text,
        [this]() { menu->setActiveOption(optNet); showNetworks(); });
    optSet = new Aether::MenuOption("Settings", theme::Accent, theme::Text,
        [this]() { menu->setActiveOption(optSet); showSettings(); });
    optDiag = new Aether::MenuOption("Diagnostics", theme::Accent, theme::Text,
        [this]() { menu->setActiveOption(optDiag); showDiagnostics(); });
    optAbout = new Aether::MenuOption("About", theme::Accent, theme::Text,
        [this]() { menu->setActiveOption(optAbout); showAbout(); });
    menu->addElement(optNet);
    menu->addElement(optSet);
    menu->addElement(optDiag);
    menu->addElement(optAbout);
    menu->setActiveOption(optNet);
    this->addElement(menu);

    // Content container (right pane).
    content = new Aether::Container(kContentX, kHeaderH + 20, kContentW, 720 - kHeaderH - 100);
    this->addElement(content);

    // Bottom control bar.
    auto *controls = new Aether::ControlBar();
    controls->addControl(Aether::Button::A, "OK");
    controls->addControl(Aether::Button::B, "Exit");
    this->addElement(controls);
    this->onButtonPress(Aether::Button::B, [this]() { this->window->exit(); });

    showNetworks();
}

void MainScreen::onUnload() {
    this->removeAllElements();
    closeMsg();
}

void MainScreen::clearContent() {
    if (content != nullptr)
        content->removeAllElements();
}

// --- Networks pane: the mode switch --------------------------------------
void MainScreen::showNetworks() {
    clearContent();
    int cy = kHeaderH + 40;

    auto *q = new Aether::Text(kContentX, cy, "Which network do you want to load?", 28);
    q->setColour(theme::Text);
    content->addElement(q);
    cy += 70;

    auto *nx = new Aether::FilledButton(kContentX, cy, 380, 150, "NEXTENDO", 34,
        [this]() { confirmSwitch(Mode::Nextendo); });
    nx->setFillColour(theme::Accent);
    nx->setTextColour(Aether::Colour(20, 20, 20, 255));
    content->addElement(nx);

    auto *nn = new Aether::BorderButton(kContentX + 400, cy, 380, 150, 4, "NINTENDO", 34,
        [this]() { confirmSwitch(Mode::Nintendo); });
    nn->setTextColour(theme::Text);
    content->addElement(nn);
    cy += 180;

    Mode m = nextendo::apply::currentMode();
    std::string status = std::string("Currently loaded: ") +
                         (m == Mode::Nextendo ? "NEXTENDO" : "NINTENDO");
    auto *st = new Aether::Text(kContentX, cy, status, 24);
    st->setColour(m == Mode::Nextendo ? theme::Accent : theme::Muted);
    content->addElement(st);
    cy += 44;

    auto *hint = new Aether::TextBlock(kContentX, cy,
        "Nextendo redirects Nintendo traffic to the Nextendo servers and reboots. "
        "Nintendo restores the official servers. Nothing is permanent.",
        20, kContentW);
    hint->setColour(theme::Muted);
    content->addElement(hint);
}

// --- Settings pane -------------------------------------------------------
void MainScreen::showSettings() {
    clearContent();
    auto *list = new Aether::List(kContentX, kHeaderH + 24, kContentW, 720 - kHeaderH - 120);

    list->addElement(new Aether::ListHeading("Server"));
    for (const auto &s : cfg::kServers) {
        std::string ip = s.ip;
        auto *opt = new Aether::ListOption(s.label, ip,
            [this, ip]() { serverIp = ip; showSettings(); });
        opt->setColours(theme::Line, theme::Muted,
                        serverIp == ip ? theme::Accent : theme::Text);
        list->addElement(opt);
    }

    list->addElement(new Aether::ListHeading("Options"));
    list->addElement(new Aether::ListOption("Language", "English",
        [this]() { /* TODO: i18n picker */ }));
    list->addElement(new Aether::ListOption("Restore my hosts in Nintendo mode", "On",
        [this]() { /* TODO: toggle backup use */ }));

    content->addElement(list);
}

// --- Diagnostics pane: mode + a live LAN server probe --------------------
void MainScreen::showDiagnostics() {
    clearContent();
    int cy = kHeaderH + 40;

    Mode m = nextendo::apply::currentMode();
    Boot b = nextendo::apply::detectBoot();
    const char *bootStr = b == Boot::Emummc ? "emuMMC" : b == Boot::Sysmmc ? "sysNAND" : "unknown";

    std::string info = std::string("Loaded mode:  ") +
        (m == Mode::Nextendo ? "NEXTENDO" : "NINTENDO") +
        "\nBoot storage: " + bootStr + "\nServer:       " + serverIp;
    auto *t = new Aether::TextBlock(kContentX, cy, info, 24, kContentW);
    t->setColour(theme::Text);
    content->addElement(t);
    cy += 130;

    auto *probeBtn = new Aether::FilledButton(kContentX, cy, 260, 60, "Test servers", 24,
        [this]() { showDiagnostics(); }); // rebuild; probe runs below on each build
    probeBtn->setFillColour(theme::Accent);
    probeBtn->setTextColour(Aether::Colour(20, 20, 20, 255));
    content->addElement(probeBtn);
    cy += 84;

    // Probe the configured server on a few well-known Nextendo ports and show
    // green/red — the answer to "is my home-lab actually up?".
    struct Svc { const char *name; int port; };
    static const Svc svcs[] = {
        {"Auth / NEX (443)", 443}, {"Demonware auth (8460)", 8460},
        {"BCAT (8470)", 8470}, {"Tagaya (8471)", 8471}, {"aauth (8473)", 8473},
    };
    for (const auto &sv : svcs) {
        bool up = ui::probe::reachable(serverIp, sv.port, 600);
        auto *line = new Aether::Text(kContentX, cy,
            std::string(up ? "●  " : "○  ") + sv.name, 22);
        line->setColour(up ? theme::Good : theme::Bad);
        content->addElement(line);
        cy += 34;
    }
}

// --- About pane ----------------------------------------------------------
void MainScreen::showAbout() {
    clearContent();
    auto *t = new Aether::TextBlock(kContentX, kHeaderH + 40,
        "nextendo-nx\n\n"
        "Switches your console between the Nextendo Network and Nintendo's "
        "servers by writing Atmosphere DNS.mitm hosts and rebooting. The heavy "
        "per-game content is delivered by the Nextendo servers, not bundled here.\n\n"
        "Built on Aether (tallbl0nde). Not affiliated with Nintendo.\n"
        "PolyForm Shield License 1.0.0 - Copyright 2026 Nextendo Network.",
        22, kContentW);
    t->setColour(theme::Text);
    content->addElement(t);
}

// --- Switch confirm + apply ---------------------------------------------
void MainScreen::confirmSwitch(Mode mode) {
    closeMsg();
    msg = new Aether::MessageBox();
    msg->setLineColour(theme::Line);
    msg->setRectangleColour(theme::Panel);
    msg->setTextColour(theme::Text);

    const bool toNx = (mode == Mode::Nextendo);
    auto *body = new Aether::TextBlock(0, 0,
        toNx ? "Switch to Nextendo? This redirects Nintendo traffic to the "
               "Nextendo servers and reboots."
             : "Switch to Nintendo? This removes everything Nextendo installed, "
               "restores the official servers and reboots.",
        22, 560);
    body->setColour(theme::Text);
    msg->setBodySize(600, 220);
    msg->setBody(body);
    msg->addLeftButton("Cancel", [this]() { closeMsg(); });
    msg->addRightButton("Switch", [this, mode]() { doSwitch(mode); });
    window->addOverlay(msg);
}

void MainScreen::doSwitch(Mode mode) {
    closeMsg();
    const bool ok = (mode == Mode::Nextendo)
                        ? nextendo::apply::applyNextendo(serverIp)
                        : nextendo::apply::applyNintendo();
    if (ok) {
        nextendo::apply::reboot(); // does not return on success
    }
    // Failure: surface it.
    msg = new Aether::MessageBox();
    msg->setLineColour(theme::Line);
    msg->setRectangleColour(theme::Panel);
    msg->setTextColour(theme::Text);
    auto *body = new Aether::TextBlock(0, 0,
        "Could not apply the change. See the trace on your SD card "
        "(sd:/switch/nextendo-nx/trace.txt).", 22, 560);
    body->setColour(theme::Text);
    msg->setBodySize(600, 200);
    msg->setBody(body);
    msg->addRightButton("OK", [this]() { closeMsg(); });
    window->addOverlay(msg);
}

void MainScreen::closeMsg() {
    if (msg != nullptr) {
        msg->close();
        delete msg;
        msg = nullptr;
    }
}

} // namespace ui
