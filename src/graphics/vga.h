#pragma once

#include "color.h"

#include <cstdint>

namespace resl {

inline static constexpr int SCREEN_WIDTH = 640;
inline static constexpr int SCREEN_HEIGHT = 350;

} // namespace resl

namespace resl::vga {

enum WriteOperation {
    Copy,
    And,
    Or,
    Xor
};

using VideoMemPtr = unsigned;

//-----------------------------------------------------------------------------

/* 15ab:00fe */
inline static constexpr unsigned VIDEO_MEM_ROW_BYTES = 0x5C;
inline static constexpr VideoMemPtr VIDEO_MEM_START_ADDR = 0xA0000; /* A000:0000 */
// entire page of the video memory A000:0000 - A000:FFFF
inline static constexpr unsigned VIDEO_MEM_N_ROWS = 0x10000 / VIDEO_MEM_ROW_BYTES;

//-----------------------------------------------------------------------------

/* 1b06:026d */
void setVideoModeR0W0();

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

/* 1b06:02b5 */
void videoChoosePlanes(std::uint8_t);

/* 1b06:03fa */
void waitVerticalRetrace();

/* 1b06:0408 */
void waitForLine(std::int16_t line);

/* 17a7:0106 */
void waitForNRetraces(std::int16_t n);

} // namespace resl::vga
