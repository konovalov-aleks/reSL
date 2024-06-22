#pragma once

#include "graphics/color.h"
#include "rail.h"
#include "types/rail_info.h"

#include <cstddef>
#include <cstdint>

namespace resl {

struct Entrance {
    Color bgColor;
    Color fgColor;
    std::uint8_t entranceRailInfoIdx;
    std::uint8_t waitingTrainsCount;
    Rail rail;
};

/*
 Only first 6 records represent real entrances.
 Last 3 are for service needs (e.g. as a destination entrance for a server train)
 Thus, they write only 6 first entrances to the game save file.
 */
static inline constexpr std::size_t NormalEntranceCount = 6;

// the special pseudo-entrance for servers
inline constexpr std::uint8_t serverEntranceIdx = 6;

// the special pseudo-entrance for blinking trains
inline constexpr std::uint8_t blinkingTrainEntranceIdx = 8;

/* 262d:5f00 : 2 bytes */
extern std::int16_t g_entranceCount;

/* 1d7d:01fc : 198 bytes */
extern Entrance g_entrances[9];

//-----------------------------------------------------------------------------

/* 17bf:03ba */
void spawnNewEntrance(RailInfo);

/* 19de:028b */
Entrance* findClosestEntrance(std::int16_t x, std::int16_t y);

/* 1a65:039d */
bool entranceIsFree(std::int16_t entranceIdx);

/* 1a65:0528 */
void addWaitingTrain(std::int16_t entranceIdx);

} // namespace resl
