#pragma once

#include "color.h"

#include <cstdint>

namespace resl::drawing {

/* 1b06:06f9 */
void filledRectangle(
    std::int16_t x8, std::int16_t y, std::int16_t width, std::int16_t height, std::uint8_t pattern,
    Color
);

/* 1b06:0956 */
void horizontalLine(std::int16_t x1, std::int16_t x2, std::int16_t y, Color);

/* 15e8:025a */
void dialogFrame(
    std::int16_t x, std::int16_t y, std::int16_t width, std::int16_t height, Color bgColor
);

/* 1abc:00a3 */
void imageDot7(
    std::int16_t x, std::int16_t y, std::int16_t width, std::int16_t height,
    const std::uint8_t* image
);

} // namespace resl::drawing
