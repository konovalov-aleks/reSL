#pragma once

#include <cstdint>

namespace resl {

struct SemaphoreGlyphBias {
    std::int8_t dx; // pixels
    std::int8_t dy; // pixels
};

/* 1d63:0000 - 24 bytes */
extern const SemaphoreGlyphBias g_semaphoreGlyphBiases[6][2]; /* [rail type][direction] */

} // namespace resl
