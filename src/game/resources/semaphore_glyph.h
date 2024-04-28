#pragma once

#include <cstdint>

namespace resl {

struct SemaphoreGlyph {
    std::int8_t xOffset;
    std::int8_t yOffset;
    std::int8_t lightXOffset;
    std::int8_t lightYOffset;
    std::uint8_t glyphBg2[15]; // white
    std::uint8_t glyphBg1[15]; // black
    std::uint8_t glyphLight[8];
};

/* 1d7d:24da : 168 bytes */
extern const SemaphoreGlyph g_semaphoreGlyphs[2][2];

} // namespace resl
