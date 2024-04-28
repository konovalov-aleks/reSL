#pragma once

#include <graphics/glyph.h>

namespace resl {

struct RailGlyph {
    std::int16_t dx;
    std::int16_t dy;
    Glyph glyph;
};

struct RailTexture {
    RailGlyph* mainGlyph;
    RailGlyph* bg2;
    RailGlyph* bg1;
    RailGlyph* switches[2]; // left / right
};

extern const RailTexture railBackgrounds[6];

} // namespace resl
