#pragma once

#include "types/semaphore.h"
#include "types/train.h"
#include <graphics/color.h>

#include <cstdint>

namespace resl {

/* 137c:006f */
void drawRailBg1(
    std::int16_t tileX, std::int16_t tileY, std::int16_t railType, Color, std::int16_t yOffset);

/* 137c:00d2 */
void drawRailBg2(
    std::int16_t tileX, std::int16_t tileY, std::int16_t railType, Color, std::int16_t yOffset);

/* 137c:0135 */
void drawSemaphore(Semaphore&, std::int16_t yOffset);

/* 17bf:0cd0 */
void drawFieldBackground(std::int16_t yOffset);

/* 18fa:08d6 */
void drawTrainList(Carriage*);

/* 15e8:09c8 */
void drawWorld();

/* 132d:004b */
void fillGameFieldBackground(std::int16_t yOffset);

/* 18fa:000b */
void scheduleTrainsDrawing();

/* 132d:0002 */
void eraseTrain(const Train&);

/* 132d:01e2 */
void drawEraseTrainFinishedExclamation(std::int16_t entranceX, std::int16_t entranceY);

/* 132d:00f9 */
void drawGameField(std::int16_t yOffset);

} // namespace resl
