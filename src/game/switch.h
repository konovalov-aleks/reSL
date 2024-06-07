#pragma once

#include "types/chunk.h"
#include "types/rail_info.h"

#include <cstdint>

namespace resl {

struct Switch {
    std::int16_t x;
    std::int16_t y;
    ChunkReference c1;
    ChunkReference curChunk;
    ChunkReference c3;
    std::int16_t x_someSwitchIndex;
};

//-----------------------------------------------------------------------------

/* 262d:21d4 : 2 bytes */
extern std::uint16_t g_nSwitches;

/* 262d:6954 : 1440 bytes */
extern Switch g_switches[80];

//-----------------------------------------------------------------------------

/* 1ad3:000c */
void createSwitches(RailInfo&);

/* 19de:00ec */
void toggleSwitch(Switch&);

/* 13d1:010f */
void drawSwitch(std::int16_t idx, bool drawToScreen);

/* 13d1:026c */
void drawSwitch2(std::int16_t idx, std::int16_t yOffset);

} // namespace resl
