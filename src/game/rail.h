#pragma once

#include "types/rail_info.h"

#include <cstdint>

namespace resl {

struct Rail;

inline static Rail* const g_disabledSwitchPath = reinterpret_cast<Rail*>(1);

// special rail type for entrances
inline static constexpr std::uint8_t g_innerEntranceRailType = 6;

struct RailConnection {
    Rail* rail;
    std::uint16_t slot;
};

struct Rail {
    std::int16_t x;
    std::int16_t y;
    std::uint8_t type;
    std::uint8_t minPathStep;
    std::uint8_t maxPathStep;
    std::int8_t semSlotIdByDirection[3];
    RailConnection connections[2];
};

//-----------------------------------------------------------------------------

/* 262d:6ef4 : 4 bytes */
extern const RailConnection g_emptyRailConnection;

/* 262d:259a : 11880 bytes */
extern Rail g_rails[11][11][6]; /* [tileX][tileY][railType] */

/* 262d:6952 : 2 bytes */
extern std::uint16_t g_railRoadCount;

/* 262d:5b02 : 900 bytes */
extern RailInfo g_railRoad[150];

/* 262d:5e86 : 121 bytes */
extern std::uint8_t g_railroadTypeMasks[11][11];

//-----------------------------------------------------------------------------

/* 19de:0426 */
std::uint16_t roadMaskInTile(std::int16_t tileX, std::int16_t tileY);

/* 19de:04a9 */
bool checkRailWouldConflict(std::int16_t tileX, std::int16_t tileY,
                            std::int16_t railType);

/* 19de:06a0 */
bool checkRailWouldConflictWithExistingRoad(std::int16_t tileX, std::int16_t tileY,
                                            std::int16_t railType);

} // namespace resl
