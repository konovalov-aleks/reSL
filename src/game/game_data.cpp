#include "game_data.h"

#include "types/rail_info.h"

namespace resl {

/* 262d:21d0 : 2 byte */
std::uint16_t g_railsLoadedOffset;

/* 262d:21ce : 2 byte */
std::uint16_t g_entrancesLoadedOffset;

/* 1d7d:01e8 : 20 byte */
char g_playerName[20];

/* 262d:6952 : 2 bytes */
std::uint16_t g_railRoadCount;

/* 262d:5b02 : 900 bytes */
RailInfo g_railRoad[150];

/* 262d:5e86 : 121 bytes */
std::uint8_t g_railroadTypeMasks[11][11];

} // namespace resl
