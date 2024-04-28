#pragma once

#include "chunk.h"

#include <cstdint>

namespace resl {

struct Switch {
    std::uint8_t f1;
    std::uint8_t f2;
    std::uint16_t f3;
    ChunkReference c1;
    ChunkReference curChunk;
    ChunkReference c3;
    std::int16_t x_someSwitchIndex;
};

} // namespace resl
