#pragma once

#include <cstdint>

namespace resl::graphics {

/* 1b06:04fd */
void shiftScreen(std::int16_t);

/* 1b06:042f */
void setVideoFrameOrigin(std::int16_t x, std::int16_t y);

/* 17a7:00cd */
void animateScreenShifting();

} // namespace resl::graphics
