#pragma once

#include "rail.h"

#include <cstdint>

namespace resl {

struct Switch {
    std::int16_t x;
    std::int16_t y;
    RailConnection exit;
    RailConnection entry;
    RailConnection disabledPath;
    // In the case of X-shaped crossing, switches are connected to each other
    // directly. This variable is the index of the adjacent switch.
    std::int16_t adjacentSwitchIdx;
};

//-----------------------------------------------------------------------------

/* 262d:21d4 : 2 bytes */
extern std::uint16_t g_nSwitches;

/* 262d:6954 : 1440 bytes */
extern Switch g_switches[80];

//-----------------------------------------------------------------------------

/* 19de:02ee */
void updateSwitchPosition(Switch&);

/* 19de:00b6 */
void configChunkStepsForSwitch(RailConnection);

/* 19de:00ec */
void toggleSwitch(Switch&);

// Draws a switch, having previously backed up the image area so that it can be
// erased by calling `eraseSwitch` function.
/* 13d1:010f */
void drawSwitch(std::int16_t idx, bool drawToScreen);

/* 13d1:0001 */
void eraseSwitch(std::int16_t idx);

// Just draws a switch, it will not be possible to erase it via `eraseSwitch` call
/* 13d1:026c */
void drawSwitchNoBackup(std::int16_t idx, std::int16_t yOffset);

/* 19de:020b */
Switch* findClosestSwitch(std::int16_t x, std::int16_t y);

// Checks whether there are trains on the switch
/* 19de:0007 */
bool switchIsBusy(const Switch&);

} // namespace resl
