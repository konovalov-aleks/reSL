#pragma once

#include "types/train.h"

#include <cstdint>

namespace resl {

/* 18a5:0145 */
bool moveTrain(Train&, std::int16_t dTime);

} // namespace resl
