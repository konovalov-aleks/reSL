#pragma once

#include "color.h"

#include <cstdint>

namespace resl {

/* 1d7d:2962 : 2 bytes */
extern std::int16_t g_textSpacing;

/* 1b06:1552 */
void drawText(std::int16_t x, std::int16_t y, const char*, Color);

/* 1b06:077c */
void drawTextSmall(std::int16_t x, std::int16_t y, const char*, Color);

} // namespace resl
