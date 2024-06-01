#pragma once

#include "chunk.h"
#include <game/resources/semaphore_glyph.h>

#include <cstdint>

namespace resl {

enum class SemaphoreType : std::int16_t {
    RightUp = 0,
    RightDown = 1,
    LeftUp = 2,
    LeftDown = 3,
    None = -1
};

struct Semaphore {
    Chunk* chunk;
    SemaphoreType type;
    bool isRed;
    bool isRightDirection;
    std::int16_t pixelX;
    std::int16_t pixelY;
    const SemaphoreGlyph* glyph;
};

} // namespace resl
