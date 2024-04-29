#pragma once

#include <cstdint>

namespace resl {

struct ImpasseGlyph {
    std::uint8_t fg[16]; // white
    std::uint8_t bg[16]; // black
};

/* 1d7d:888b - 64 bytes */
extern const ImpasseGlyph g_impasseGlyphs[2];

} // namespace resl
