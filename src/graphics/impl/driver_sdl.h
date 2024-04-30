#pragma once

#include <cstdint>

namespace resl {

inline static constexpr int SCREEN_WIDTH = 640;
inline static constexpr int SCREEN_HEIGHT = 350;

/* 15ab:00fe */
inline static constexpr unsigned VIDEO_MEM_ROW_BYTES = 0x5C;
inline static constexpr unsigned VIDEO_MEM_START_ADDR = 0xE000;

extern std::uint8_t g_videoWriteMask;
extern std::uint8_t g_videoWriteMode;
extern std::uint8_t g_videoRegMapMask;
extern std::uint8_t g_videoReadPlane;

inline void setVideoMask(std::uint8_t mask)
{
    g_videoWriteMask = mask;
}
inline void setVideoWriteMode(std::uint8_t mode)
{
    g_videoWriteMode = mode;
}
inline void setVideoMapMask(std::uint8_t mask)
{
    g_videoRegMapMask = mask;
}
inline void setVideoReadPlane(std::uint8_t p)
{
    g_videoReadPlane = p;
}
void writeVideoMem(unsigned memPtr, std::uint8_t color);
std::uint8_t readVideoMem(unsigned memPtr);

} // namespace resl
