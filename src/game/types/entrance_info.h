#pragma once

#include "chunk.h"
#include "graphics/color.h"

#include <cstdint>

namespace resl {

struct EntranceInfo {
    Color bgColor;
    Color fgColor;
    //std::uint8_t tileX;
    std::uint8_t tileY;
    std::uint8_t waitingTrainsCount;
    Chunk chunk;
};

} // namespace resl
