#include "semaphore_glyph_bias.h"

namespace resl {

/* 1d63:0000 - 24 bytes */
const SemaphoreGlyphBias g_semaphoreGlyphBias[6][2] = {
    { { -9, -3 }, { 9, 3 } },
    { { 9, -3 }, { -9, 3 } },
    { { -9, -3 }, { -9, 3 } },
    { { 9, -3 }, { -9, -3 } },
    { { 9, -3 }, { 9, 3 } },
    { { 9, 3 }, { -9, 3 } },
};

} // namespace resl
