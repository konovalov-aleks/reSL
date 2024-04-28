#pragma once

#include "chunk.h"

#include <cstdint>

namespace resl {

struct EntranceInfo {
    bool dispathcerSignalling;
    std::uint8_t tileX;
    std::uint8_t tileY;
    std::uint8_t waitingTrainsCount;
    Chunk chunk;
};

} // namespace resl
