#pragma once

#include <cstdint>

namespace resl {

enum WriteOperation {
    Copy,
    And,
    Or,
    Xor
};

using VideoMemPtr = unsigned;

inline static constexpr int SCREEN_WIDTH = 640;
inline static constexpr int SCREEN_HEIGHT = 350;

/* 15ab:00fe */
inline static constexpr unsigned VIDEO_MEM_ROW_BYTES = 0x5C;
inline static constexpr VideoMemPtr VIDEO_MEM_START_ADDR = 0xA0000; /* A000:0000 */
// entire page of the video memory A000:0000 - A000:FFFF
inline static constexpr unsigned VIDEO_MEM_N_ROWS = 0x10000 / VIDEO_MEM_ROW_BYTES + 1;

} // namespace resl
