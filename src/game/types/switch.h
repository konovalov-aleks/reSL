#pragma once

#include "chunk.h"

#include <cstdint>

namespace resl {

struct Switch {
    std::int16_t x;
    std::int16_t y;
    ChunkReference c1;
    ChunkReference curChunk;
    ChunkReference c3;
    std::int16_t x_someSwitchIndex;
};

} // namespace resl
