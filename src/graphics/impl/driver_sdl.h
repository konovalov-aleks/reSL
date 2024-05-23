#pragma once

#include <cstdint>

namespace resl {

// https://www.cs.utexas.edu/~lorenzo/corsi/439/ref/hardware/vgadoc/VGAREGS.TXT

using VideoMemPtr = unsigned;

inline static constexpr int SCREEN_WIDTH = 640;
inline static constexpr int SCREEN_HEIGHT = 350;

/* 15ab:00fe */
inline static constexpr unsigned VIDEO_MEM_ROW_BYTES = 0x5C;
inline static constexpr VideoMemPtr VIDEO_MEM_START_ADDR = 0xA0000; /* A000:0000 */
inline static constexpr unsigned VIDEO_MEM_N_ROWS = SCREEN_HEIGHT * 2;

enum WriteOperation {
    Copy,
    And,
    Or,
    Xor
};

void setVideoMask(std::uint8_t);
void setVideoMode(std::uint8_t);
void setVideoMapMask(std::uint8_t);
void setVideoReadPlane(std::uint8_t);
void setVideoReadPlane(std::uint8_t);
void setVideoWriteOperation(WriteOperation);

std::uint8_t videoWriteMode();

void writeVideoMem(VideoMemPtr memPtr, std::uint8_t color);
std::uint8_t readVideoMem(VideoMemPtr memPtr);

} // namespace resl
