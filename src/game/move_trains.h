#pragma once

#include "train.h"
#include <tasks/task.h>

#include <cstdint>

namespace resl {

/* 18a5:00c1 */
void moveAlongPath(Location& loc, std::int16_t distance);

/* 18a5:0145 */
bool moveTrain(Train&, std::int16_t dTime);

/* 18fa:068a */
Task taskMoveAndRedrawTrains();

} // namespace resl
