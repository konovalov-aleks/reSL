#pragma once

#include "types/rail_info.h"

#include <cstdint>

namespace resl {

/* 262d:21d0 : 2 byte */
extern std::uint16_t chunksLoadedOffset;

/* 262d:21ce : 2 byte */
extern std::uint16_t entrancesLoadedOffset;

/* 1d7d:01e8 : 20 byte */
extern char playerName[20];

/* 262d:6952 : 2 bytes */
extern std::uint16_t g_railRoadCount;

/* 262d:5b02 : 900 bytes */
extern RailInfo g_railRoad[150];

/* 262d:5e86 : 121 bytes */
extern std::uint8_t railroadTypeMasks[11][11];

} // namespace resl
