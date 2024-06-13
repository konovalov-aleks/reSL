#pragma once

#include "color.h"
#include "vga.h"
#include <game/types/rectangle.h>

#include <cstdint>

namespace resl::drawing {

/* 1b06:0279 */
void setVideoModeR0W1();

/* 1b06:0285 */
void setVideoModeR0W2();

// The function in the original game receives an uint8 value here (VGA color code)
/* 1b06:02ec */
void setPaletteItem(Color, std::uint32_t rgb);

/* Configures the data rotation register

   bit 0-2  Number of positions to rotate data right before it is written to
            display memory. Only active in Write Mode 0.
       3-4  In Write Mode 2 this field controls the relation between the data
            written from the CPU, the data latched from the previous read and the
            data written to display memory:
              0: CPU Data is written unmodified
              1: CPU data is ANDed with the latched data
              2: CPU data is ORed  with the latch data.
              3: CPU data is XORed with the latched data. */
/* 1b06:02a5 */
void setDataRotation(std::uint8_t);

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

/* 1b06:086a */
void line(std::int16_t x1, std::int16_t y1, std::int16_t x2, std::int16_t y2, Color);

/* 1b06:0324 */
void copyRectangle(std::int16_t dstX, std::int16_t dstY,
                   std::int16_t srcX, std::int16_t srcY,
                   std::int16_t width, std::int16_t height);

/* The block of video memory starting at A000:7DC8 is used as a shadow buffer

   0x7DC8 = VIDEO_MEM_ROW_BYTES<0x5C> * SCREEN_HEIGHT<350>

*/
inline static constexpr VideoMemPtr VIDEO_MEM_SHADOW_BUFFER = VIDEO_MEM_START_ADDR + 0x7DC8; /* A000:7DC8 */

/* 1b06:0004 */
void copyFromShadowBuffer(const Rectangle&);

/* 1b06:0062 */
VideoMemPtr copySpriteToShadowBuffer(VideoMemPtr, std::int16_t x, std::int16_t y,
                                     std::int16_t width, std::int16_t height);

/* 1b06:00a0 */
VideoMemPtr copySpriteFromShadowBuffer(VideoMemPtr, std::int16_t x, std::int16_t y,
                                       std::int16_t width, std::int16_t height);

/* 1b06:0379 */
void saveVideoMemRegion24x16(std::int16_t x, std::int16_t y, VideoMemPtr dst);

/* 1b06:03bb */
void restoreVideoMemRegion24x16(std::int16_t x, std::int16_t y, VideoMemPtr src);

} // namespace resl::drawing
