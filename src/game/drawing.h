#pragma once

#include "rail.h"
#include "types/rectangle.h"
#include <graphics/color.h>

#include <cstdint>

namespace resl {

/* 262d:6f94 : 8 bytes */
extern Rectangle g_areaToRedraw;

//-----------------------------------------------------------------------------

// copies the area specified by g_areaToRedraw from the shadow buffer to the screen
/* 17bf:0e1b */
void redrawScreenArea();

/* 17bf:0599 */
void scheduleRailRedrawing(const Rail&);

/* 17bf:05dc */
void clampRectToGameFieldBoundaries(Rectangle&);

/* 137c:006f */
void drawRailBg1(
    std::int16_t tileX, std::int16_t tileY, std::int16_t railType, Color, std::int16_t yOffset);

/* 137c:00d2 */
void drawRailBg2(
    std::int16_t tileX, std::int16_t tileY, std::int16_t railType, Color, std::int16_t yOffset);

/* 17bf:0cd0 */
void drawFieldBackground(std::int16_t yOffset);

/* 15e8:09c8 */
void drawWorld();

/* 132d:004b */
void fillGameFieldBackground(std::int16_t yOffset);

/* 132d:01e2 */
void drawEraseTrainFinishedExclamation(std::int16_t entranceX, std::int16_t entranceY);

/* 132d:00f9 */
void drawGameField(std::int16_t yOffset);

} // namespace resl
