// nextendo-nx — main screen (menu + panes).
// Copyright (C) 2026 Nextendo Network. PolyForm Shield License 1.0.0.
// Required Notice: Copyright 2026 Nextendo Network
#include "ui/MainScreen.hpp"
#include "ui/theme.hpp"
#include "ui/probe.hpp"
#include "nextendo/config.hpp"
#include "nextendo/hosts.hpp"
#include "nextendo/news.hpp"
#include "nextendo/users.hpp"

#include <switch.h>
#include <string>
#include <vector>

namespace ui {

namespace cfg = nextendo::config;
using nextendo::apply::Mode;
using nextendo::apply::Boot;

static Aether::Element *centredBody(const std::string &text, unsigned int size, int w, int h);

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
    optUsers = new Aether::MenuOption("Users", theme::Accent, theme::Text,
        [this]() { menu->setActiveOption(optUsers); showUsers(); });
    optDiag = new Aether::MenuOption("Diagnostics", theme::Accent, theme::Text,
        [this]() { menu->setActiveOption(optDiag); showDiagnostics(); });
    optAbout = new Aether::MenuOption("About", theme::Accent, theme::Text,
        [this]() { menu->setActiveOption(optAbout); showAbout(); });
    menu->addElement(optNet);
    menu->addElement(optSet);
    menu->addElement(optUsers);
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
    nx->setTextColour(theme::OnAccent);
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

// --- Users pane: link state; unlink locally (then delete in System Settings) ---
void MainScreen::showUsers() {
    clearContent();
    auto *list = new Aether::List(kContentX, kHeaderH + 24, kContentW, 720 - kHeaderH - 120);
    list->addElement(new Aether::ListHeading("Users"));
    std::string err;
    auto users = nextendo::users::list(err);
    if (!err.empty()) {
        auto *t = new Aether::ListComment("Could not list users: " + err);
        list->addElement(t);
    }
    for (const auto &u : users) {
        const AccountUid uid = u.uid;
        const std::string name = u.nickname;
        auto *opt = new Aether::ListOption(name, u.linked ? "Linked" : "Offline", [this, uid, name, linked = u.linked]() {
            if (!linked) {
                showInfo(name + " has no Nintendo Account link. Delete it in System Settings > Users.");
                return;
            }
            closeMsg();
            msg = new Aether::MessageBox();
            msg->setLineColour(theme::Line);
            msg->setRectangleColour(theme::Panel);
            msg->setTextColour(theme::Text);
            msg->setBodySize(600, 220);
            msg->setBody(centredBody("Unlink " + name + "'s Nintendo Account on this console? Its saves stay. "
                                     "Then it can be deleted in System Settings > Users.", 22, 600, 220));
            msg->addLeftButton("Cancel", [this]() { closeMsg(); });
            msg->addRightButton("Unlink", [this, uid, name]() {
                std::string e;
                if (!nextendo::users::unlinkLocally(uid, e)) {
                    showInfo("Could not unlink " + name + ": " + e);
                    return;
                }
                // The account service keeps the old link state until it restarts: reboot, then delete.
                closeMsg();
                msg = new Aether::MessageBox();
                msg->setLineColour(theme::Line);
                msg->setRectangleColour(theme::Panel);
                msg->setTextColour(theme::Text);
                msg->setBodySize(600, 220);
                msg->setBody(centredBody(name + " is unlinked on this console. Reboot, then delete it in "
                                         "System Settings > Users.", 22, 600, 220));
                msg->addLeftButton("Later", [this]() { closeMsg(); });
                msg->addRightButton("Reboot", []() { nextendo::apply::reboot(); });
                window->addOverlay(msg);
            });
            window->addOverlay(msg);
        });
        opt->setColours(theme::Line, u.linked ? theme::Accent : theme::Muted, theme::Text);
        list->addElement(opt);
    }
    list->addElement(new Aether::ListComment(
        "Unlink removes the Nintendo Account link on this console only, for users a failed link left half-linked "
        "(\"unable to use\", 2002-0001 on delete). Nothing is sent to a server."));
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
    probeBtn->setTextColour(theme::OnAccent);
    content->addElement(probeBtn);

    // Copies the console's news to the SD card (read-only): the reference format for Nextendo news.
    auto *newsBtn = new Aether::BorderButton(kContentX + 280, cy, 250, 60, 3, "Dump news", 24, [this]() {
        std::string err;
        int n = nextendo::news::dump(err);
        showInfo(n < 0 ? "Could not read the news: " + err
                       : std::to_string(n) + " news item(s) copied to sd:/switch/nextendo-nx/news/");
    });
    newsBtn->setTextColour(theme::Text);
    content->addElement(newsBtn);

    // Posts the news records in sd:/switch/nextendo-nx/news/post/ as local news (no network, no signature).
    auto *postBtn = new Aether::BorderButton(kContentX + 550, cy, 250, 60, 3, "Post news", 24, [this]() {
        std::string err;
        int n = nextendo::news::post(err);
        showInfo(n < 0 ? "Could not post news: " + err
                       : std::to_string(n) + " news item(s) posted. Results in sd:/switch/nextendo-nx/news/post.txt");
    });
    postBtn->setTextColour(theme::Text);
    content->addElement(postBtn);
    cy += 84;

    // Removes all news from the console (Nintendo's notices too), after a confirmation.
    auto *clearBtn = new Aether::BorderButton(kContentX + 280, cy, 250, 60, 3, "Clear news", 24, [this]() {
        closeMsg();
        msg = new Aether::MessageBox();
        msg->setLineColour(theme::Line);
        msg->setRectangleColour(theme::Panel);
        msg->setTextColour(theme::Text);
        msg->setBodySize(600, 220);
        msg->setBody(centredBody("Remove ALL news from this console, Nintendo's notices included? "
                                 "Use Dump news first to keep a copy.", 22, 600, 220));
        msg->addLeftButton("Cancel", [this]() { closeMsg(); });
        msg->addRightButton("Clear", [this]() {
            std::string err;
            bool ok = nextendo::news::clear(err);
            showInfo(ok ? "All news removed; the News channels are fetched again from the server."
                        : "Could not clear the news: " + err);
        });
        window->addOverlay(msg);
    });
    clearBtn->setTextColour(theme::Text);
    content->addElement(clearBtn);

    // Subscribes the default News channels and fetches them now from the server.
    auto *getBtn = new Aether::BorderButton(kContentX + 550, cy, 250, 60, 3, "Get news", 24, [this]() {
        std::string err;
        std::string res = nextendo::news::fetch(err);
        showInfo(res.empty() ? "Could not get news: " + err : "Subscribed and requested:\n" + res);
    });
    getBtn->setTextColour(theme::Text);
    content->addElement(getBtn);
    cy += 84;

    // Probe the configured server on a few well-known Nextendo ports and show
    // green/red — the answer to "is my home-lab actually up?".
    struct Svc { const char *name; int port; };
    static const Svc svcs[] = {
        {"Auth / NEX (443)", 443}, {"Demonware auth (8460)", 8460},
        {"BCAT (8470)", 8470}, {"Tagaya (8471)", 8471}, {"dauth + aauth (8446)", 8446},
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

// A message-box body of w x h with the text wrapped to fit and every line centred, horizontally and vertically.
// Aether pins a body to the box's top-left and has no centred text block, so the lines are laid out here.
static Aether::Element *centredBody(const std::string &text, unsigned int size, int w, int h) {
    const int maxW = w - 60;
    std::vector<Aether::Text *> lines;
    std::string line, word;
    auto flush = [&](const std::string &s) {
        auto *t = new Aether::Text(0, 0, s, size);
        t->setColour(theme::Text);
        lines.push_back(t);
    };
    auto fits = [&](const std::string &s) {
        Aether::Text probe(0, 0, s, size);
        return probe.w() <= maxW;
    };
    for (size_t i = 0; i <= text.size(); i++) {
        if (i == text.size() || text[i] == ' ') {
            const std::string candidate = line.empty() ? word : line + " " + word;
            if (!line.empty() && !fits(candidate)) { flush(line); line = word; }
            else { line = candidate; }
            word.clear();
        } else {
            word += text[i];
        }
    }
    if (!line.empty()) { flush(line); }

    auto *body = new Aether::Element(0, 0, w, h);
    const int lineH = lines.empty() ? 0 : lines[0]->h() + 6;
    int y = (h - lineH * static_cast<int>(lines.size())) / 2;
    for (auto *t : lines) {
        t->setXY((w - t->w()) / 2, y);
        body->addElement(t);
        y += lineH;
    }
    // A separator between the text and the buttons below it.
    auto *rule = new Aether::Rectangle(0, h - 2, w, 2);
    rule->setColour(theme::Muted);
    body->addElement(rule);
    return body;
}

// --- Switch confirm + apply ---------------------------------------------
void MainScreen::confirmSwitch(Mode mode) {
    closeMsg();
    msg = new Aether::MessageBox();
    msg->setLineColour(theme::Line);
    msg->setRectangleColour(theme::Panel);
    msg->setTextColour(theme::Text);

    const bool toNx = (mode == Mode::Nextendo);
    msg->setBodySize(600, 220);
    msg->setBody(centredBody(
        toNx ? "Switch to Nextendo? This redirects Nintendo traffic to the "
               "Nextendo servers and reboots."
             : "Switch to Nintendo? This removes everything Nextendo installed, "
               "restores the official servers and reboots.",
        22, 600, 220));
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
    msg->setBodySize(600, 200);
    msg->setBody(centredBody(
        "Could not apply the change. See the trace on your SD card "
        "(sd:/switch/nextendo-nx/trace.txt).", 22, 600, 200));
    msg->addRightButton("OK", [this]() { closeMsg(); });
    window->addOverlay(msg);
}

void MainScreen::showInfo(const std::string &text) {
    closeMsg();
    msg = new Aether::MessageBox();
    msg->setLineColour(theme::Line);
    msg->setRectangleColour(theme::Panel);
    msg->setTextColour(theme::Text);
    msg->setBodySize(600, 200);
    msg->setBody(centredBody(text, 22, 600, 200));
    msg->addRightButton("OK", [this]() { closeMsg(); });
    window->addOverlay(msg);
}

void MainScreen::closeMsg() {
    if (msg != nullptr) {
        msg->close();
        closing.emplace_back(msg, 0);
        msg = nullptr;
    }
}

void MainScreen::update(unsigned int dt) {
    Aether::Screen::update(dt);
    // A modal closed this frame is dropped by the Window after this update: free it on the next one.
    for (auto it = closing.begin(); it != closing.end();) {
        if (it->second++ >= 1) {
            delete it->first;
            it = closing.erase(it);
        } else {
            ++it;
        }
    }
}

} // namespace ui
