#pragma once

#include <cstdint>

namespace resl {

struct StaticObjectGlyph {
    std::uint8_t fg[32]; // black
    std::uint8_t bg[32]; // colored
};

/* 1d7d:88cb - 320 bytes */
extern const StaticObjectGlyph g_houseGlyphs[5];

/* 1d7d:8a0b - 256 bytes */
extern const StaticObjectGlyph g_treeGlyphs[4];

} // namespace resl
