#pragma once

#include "color.h"
#include "driver.h"
#include <game/types/rectangle.h>

#include <cstdint>

namespace resl::drawing {

/* 1b06:0279 */
void setVideoModeR0W1();

/* 1b06:0285 */
void setVideoModeR0W2();

/* 1b06:06f9 */
void filledRectangle(std::int16_t x8, std::int16_t y,
                     std::int16_t width, std::int16_t height,
                     std::uint8_t pattern, Color);

/* 1b06:0956 */
void horizontalLine(std::int16_t x1, std::int16_t x2, std::int16_t y, Color);

/* 15e8:025a */
void dialogFrame(
    std::int16_t x, std::int16_t y, std::int16_t width, std::int16_t height, Color bgColor);

/* 1abc:00a3 */
void imageDot7(std::int16_t x, std::int16_t y,
               std::int16_t width, std::int16_t height,
               const std::uint8_t* image);

/* 1b06:07db */
Color getPixel(std::int16_t x, std::int16_t y);

/* 1b06:0838 */
void putPixel(std::int16_t x, std::int16_t y, Color);

/* 1b06:0324 */
void copyRectangle(std::int16_t dstX, std::int16_t dstY,
                   std::int16_t srcX, std::int16_t srcY,
                   std::int16_t width, std::int16_t height);

/* The block of video memory starting at A000:7DC8 is used as a shadow buffer

   0x7DC8 = VIDEO_MEM_ROW_BYTES<0x5C> * SCREEN_HEIGHT<350>

*/
inline static constexpr VideoMemPtr VIDEO_MEM_SHADOW_BUFFER = 0x7DC8; /* A000:7DC8 */

/* 1b06:0004 */
void copyFromShadowBuffer(const Rectangle&);

/* 1b06:0062 */
VideoMemPtr copySpriteToShadowBuffer(VideoMemPtr, std::int16_t x, std::int16_t y,
                                     std::int16_t width, std::int16_t height);

/* 1b06:00a0 */
VideoMemPtr copySpriteFromShadowBuffer(VideoMemPtr, std::int16_t x, std::int16_t y,
                                       std::int16_t width, std::int16_t height);

} // namespace resl::drawing
