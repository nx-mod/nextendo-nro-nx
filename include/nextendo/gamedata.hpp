// nextendo-nx — the installed games' BCAT (delivery cache) settings.
// Copyright (C) 2026 Nextendo Network. PolyForm Shield License 1.0.0.
#pragma once
#include <string>

namespace nextendo::gamedata {

// Writes every installed game's BCAT settings, from its control data (NACP), to sd:/switch/nextendo-nx/bcat/:
//   titles.txt       title id, name, delivery cache size, whether it has a BCAT passphrase
//   passphrases.txt  title id = passphrase, for the games that have one (what bcat-nx needs to serve a game's
//                    data: keep it on the card, it is the game's key)
// Read-only on the console. Returns how many games use BCAT, or -1 with `err` set; `total` is all games seen.
int dump(int &total, std::string &err);

} // namespace nextendo::gamedata
