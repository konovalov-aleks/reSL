#pragma once

#include "chunk.h"
#include "graphics/color.h"

#include <cstddef>
#include <cstdint>

namespace resl {

struct EntranceInfo {
    Color bgColor;
    Color fgColor;
    std::uint8_t unknown;
    std::uint8_t waitingTrainsCount;
    Chunk chunk;
};

/*
 Only first 6 records represent real entrances.
 Last 3 are for service needs (e.g. as a destination entrance for a server train)
 Thus, they write only 6 first entrances to the game save file.
 */
static inline constexpr std::size_t NormalEntranceCount = 6;

/* 262d:5f00 : 2 bytes */
extern std::int16_t g_entranceCount;

/* 1d7d:01fc : 198 bytes */
extern EntranceInfo g_entrances[9];

} // namespace resl
