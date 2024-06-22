#pragma once

#include "rail.h"

#include <cstdint>

namespace resl {

/* 137c:024d */
void drawImpasse(const Rail&, std::int16_t yOffset);

/* 137c:0378 */
void eraseImpasse(const Rail&, std::int16_t yOffset);

} // namespace resl
