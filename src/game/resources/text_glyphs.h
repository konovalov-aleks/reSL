#pragma once

#include <cstdint>

namespace resl {

struct TextGlyph {
    std::int16_t yOffset;
    std::int16_t height;
    std::int16_t width;
    const std::uint8_t* glyph;
};

//-----------------------------------------------------------------------------

/* 1d7d:2683 : 735 bytes */
extern const TextGlyph g_textGlyphs[147];

} // namespace resl
