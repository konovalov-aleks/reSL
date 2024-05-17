#pragma once

#include "chunk.h"
#include "graphics/color.h"

#include <cstddef>
#include <cstdint>

namespace resl {

struct EntranceInfo {
    Color bgColor;
    Color fgColor;
    // std::uint8_t tileX;
    std::uint8_t tileY;
    std::uint8_t waitingTrainsCount;
    Chunk chunk;
};

/*
 Only first 6 records represent real entrances.
 Last 3 are for service needs (e.g. as a destination entrance for a server train)
 Thus, they write only 6 first entrances to the game save file.
 */
static inline constexpr std::size_t NormalEntranceCount = 6;

} // namespace resl
