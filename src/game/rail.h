#pragma once

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

/* 262d:259a : 11880 bytes */
extern Rail g_rails[11][11][6]; /* [tileX][tileY][railType] */

/* 262d:6ef4 : 4 bytes */
extern const RailConnection g_emptyRailConnection;

} // namespace resl
