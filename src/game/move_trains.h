#pragma once

#include "types/train.h"
#include <tasks/task.h>

#include <cstdint>

namespace resl {

/* 18a5:0145 */
bool moveTrain(Train&, std::int16_t dTime);

/* 18fa:068a */
Task taskMoveAndRedrawTrains();

} // namespace resl
