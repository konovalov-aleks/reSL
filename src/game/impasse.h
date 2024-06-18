#pragma once

#include "chunk.h"

#include <cstdint>

namespace resl {

/* 137c:024d */
void drawImpasse(const Chunk&, std::int16_t yOffset);

/* 137c:0378 */
void eraseImpasse(const Chunk&, std::int16_t yOffset);

} // namespace resl
