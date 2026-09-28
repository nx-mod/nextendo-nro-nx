// nextendo-nx — colour palette.
// Copyright (C) 2026 Nextendo Network. PolyForm Shield License 1.0.0.
#pragma once
#include <Aether/Aether.hpp>
#include <switch.h>

// The console's own look: init() reads the system theme (Basic White or Basic Black, System Settings > Themes)
// and takes Aether's matching Horizon palette, the colours the HOME menu uses.
namespace ui::theme {
inline Aether::Theme_T System = Aether::Theme::Dark;

inline Aether::Colour Background  = System.bg;
inline Aether::Colour Panel       = System.altBG;
inline Aether::Colour HighlightBg = System.highlightBG;
inline Aether::Colour Accent      = System.accent;
inline Aether::Colour AccentDim   = System.mutedLine;
inline Aether::Colour OnAccent    { 20, 20, 20, 255 };   // text drawn on an Accent fill
inline Aether::Colour Text        = System.text;
inline Aether::Colour Muted       = System.mutedText;
inline Aether::Colour Line        = System.mutedLine;
inline const Aether::Colour Good     { 80, 200, 120, 255 };
inline const Aether::Colour Bad      { 220, 90, 80, 255 };
inline const Aether::Colour Nintendo { 230, 0, 18, 255 };   // Nintendo red

inline void init() {
    ColorSetId id = ColorSetId_Dark;
    if (R_SUCCEEDED(setsysInitialize())) {
        setsysGetColorSetId(&id);
        setsysExit();
    }
    const bool light = (id == ColorSetId_Light);
    System      = light ? Aether::Theme::Light : Aether::Theme::Dark;
    Background  = System.bg;
    Panel       = System.altBG;
    HighlightBg = System.highlightBG;
    Accent      = System.accent;
    AccentDim   = System.mutedLine;
    OnAccent    = light ? Aether::Colour{255, 255, 255, 255} : Aether::Colour{20, 20, 20, 255};
    Text        = System.text;
    Muted       = System.mutedText;
    Line        = System.mutedLine;
}
}
