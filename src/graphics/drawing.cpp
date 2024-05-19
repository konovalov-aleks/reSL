#include "drawing.h"

#include "driver.h"

#include <cassert>
#include <utility>

namespace resl::drawing {

static std::uint16_t ror16(std::uint16_t x, std::uint8_t count)
{
    return (x >> count) | (x << (16 - count));
}

/* 1b06:026d */
static void setVideoModeR0W0()
{
    setVideoMode(0);
    setVideoMask(0xFF);
}

/* 1b06:0279 */
void setVideoModeR0W1()
{
    setVideoMode(1);
    setVideoMask(0xFF);
}

/* 1b06:0285 */
void setVideoModeR0W2()
{
    setVideoMode(2);
    setVideoMask(0xFF);
}

/* 1b06:02b5 */
static void videoChoosePlanes(std::uint8_t mask)
{
    setVideoMapMask(mask);
}

/* 1b06:06f9 */
void filledRectangle(std::int16_t x, std::int16_t y,
                     std::int16_t width, std::int16_t height,
                     std::uint8_t pattern, Color color)
{
    assert(g_videoWriteMode == 2);
    VideoMemPtr videoPtr = VIDEO_MEM_START_ADDR + y * VIDEO_MEM_ROW_BYTES + (x / 8);
    setVideoMask(pattern);
    for (int curY = 0; curY < height; ++curY) {
        VideoMemPtr vp = videoPtr;
        for (std::int16_t curX = 0; curX < width; ++curX)
            writeVideoMem(vp++, color);
        videoPtr += VIDEO_MEM_ROW_BYTES;
    }
}

/* 1b06:0956 */
void horizontalLine(std::int16_t x1, std::int16_t x2, std::int16_t y, Color color)
{
    if (x2 < x1)
        std::swap(x1, x2);

    VideoMemPtr videoPtr1 = VIDEO_MEM_START_ADDR + y * VIDEO_MEM_ROW_BYTES + (x1 / 8);
    VideoMemPtr videoPtr2 = VIDEO_MEM_START_ADDR + y * VIDEO_MEM_ROW_BYTES + (x2 / 8);

    const std::uint8_t pixelOffsetInsideByte1 = x1 & 7;
    const std::uint8_t pixelOffsetInsideByte2 = x2 & 7;

    if (videoPtr1 == videoPtr2) {
        // single point
        setVideoMask(
            (ror16(0x80FF, pixelOffsetInsideByte2) >> 8) & (0xFF >> pixelOffsetInsideByte1) & 0xFF);
        writeVideoMem(videoPtr1, color);
    } else {
        setVideoMask(0xFF >> pixelOffsetInsideByte1);
        writeVideoMem(videoPtr1++, color);

        setVideoMask(0xFF);
        for (std::int16_t i = 0, cnt = videoPtr2 - videoPtr1; i < cnt; ++i)
            writeVideoMem(videoPtr1++, color);

        setVideoMask((ror16(0x80FF, pixelOffsetInsideByte2) >> 8) & 0xFF);
        writeVideoMem(videoPtr1++, color);
    }
}

/* 15e8:025a */
void dialogFrame(std::int16_t x, std::int16_t y,
                 std::int16_t width, std::int16_t height, Color bgColor)
{
    const std::int16_t x2 = x + (width + -1) * 8;
    const std::int16_t roundedX1 = x & 0xfff8; // 1111111111111000b
    const std::int16_t roundedX2 = roundedX1 + width * 8;
    filledRectangle(x, y, width, height, 0xff, bgColor);
    filledRectangle(x, y, 1, height, 0x60, Color::White);
    filledRectangle(x, y, 1, height, 0x90, Color::Black);
    filledRectangle(x2, y, 1, height, 6, Color::White);
    filledRectangle(x2, y, 1, height, 9, Color::Black);
    filledRectangle(x, y, width, 1, 0xff, Color::Black);
    filledRectangle(x, y + height, width, 1, 0xff, Color::Black);
    horizontalLine(roundedX1 + 1, roundedX2 + -2, y + 1, Color::White);
    horizontalLine(roundedX1 + 1, roundedX2 + -2, y + height + -1, Color::White);
    horizontalLine(roundedX1 + 3, roundedX2 + -4, y + 2, Color::Black);
    horizontalLine(roundedX1 + 3, roundedX2 + -4, y + height + -2, Color::Black);
}

/* 1b06:00de */
static void implDrawImageDot7(std::int16_t x, std::int16_t y,
                              const std::uint8_t* data, bool drawSprite)
{
    VideoMemPtr videoPtr = VIDEO_MEM_START_ADDR + y * VIDEO_MEM_ROW_BYTES + (x / 8);
    const int nRows = 7;
    if (drawSprite) {
        // copy sprite data
        for (int r = 0; r < nRows; ++r) {
            writeVideoMem(videoPtr, *(data++));
            videoPtr += VIDEO_MEM_ROW_BYTES;
        }
    } else {
        // fill with a constant
        const std::uint8_t v = *data;
        for (int r = 0; r < nRows; ++r) {
            writeVideoMem(videoPtr, v);
            videoPtr += VIDEO_MEM_ROW_BYTES;
        }
    }
}

/* 1abc:00a3 */
void imageDot7(std::int16_t x, std::int16_t y,
               std::int16_t width, std::int16_t height,
               const std::uint8_t* image)
{
    const std::uint8_t* data = &image[0x10];
    setVideoModeR0W0();
    for (std::int16_t curY = 0; curY < height; curY += 7) {
        for (std::int16_t curX = 0; curX < 640; curX += 8) {
            std::uint8_t drawed = 0;
            while (drawed != 0xF) {
                const std::uint8_t b = *data;
                const std::uint8_t planes = b & 0xF;
                drawed |= planes;
                videoChoosePlanes(planes);
                ++data;
                if (curX < width)
                    implDrawImageDot7(x + curX, y + curY, data, (b & 0xF0) != 0);
                data += (b & 0xF0) ? 7 : 1;
            }
        }
    }
    setVideoModeR0W2();
    videoChoosePlanes(0xF);
}

/* 1b06:07db */
Color getPixel(std::int16_t x, std::int16_t y)
{
    const VideoMemPtr videoPtr = VIDEO_MEM_START_ADDR + y * VIDEO_MEM_ROW_BYTES + (x / 8);
    const std::uint8_t mask = 0x80 >> (x & 7);

    setVideoReadPlane(0);
    std::uint8_t res = (readVideoMem(videoPtr) & mask) != 0;

    setVideoReadPlane(1);
    if (readVideoMem(videoPtr) & mask)
        res |= 2;

    setVideoReadPlane(2);
    if (readVideoMem(videoPtr) & mask)
        res |= 4;

    setVideoReadPlane(3);
    if (readVideoMem(videoPtr) & mask)
        res |= 8;

    return static_cast<Color>(res);
}

/* 1b06:0838 */
void putPixel(std::int16_t x, std::int16_t y, Color c)
{
    const VideoMemPtr videoPtr = VIDEO_MEM_START_ADDR + y * VIDEO_MEM_ROW_BYTES + (x / 8);

    setVideoMask(0x80 >> (x & 7));
    writeVideoMem(videoPtr, c);

    assert(getPixel(x, y) == c);
}

/* 1b06:0324 */
void copyRectangle(std::int16_t dstX, std::int16_t dstY,
                   std::int16_t srcX, std::int16_t srcY,
                   std::int16_t width, std::int16_t height)
{
    setVideoMode(1);

    VideoMemPtr srcVideoPtr = VIDEO_MEM_START_ADDR + srcY * VIDEO_MEM_ROW_BYTES + (srcX / 8);
    VideoMemPtr dstVideoPtr = VIDEO_MEM_START_ADDR + dstY * VIDEO_MEM_ROW_BYTES + (dstX / 8);

    for (std::int16_t y = 0; y < height; ++y) {
        for (std::int16_t x = 0; x < width; ++x) {
            std::uint8_t data = readVideoMem(srcVideoPtr++);
            writeVideoMem(dstVideoPtr++, data);
        }
        srcVideoPtr += VIDEO_MEM_ROW_BYTES - width;
        dstVideoPtr += VIDEO_MEM_ROW_BYTES - width;
    }
}

/* 1b06:0004 */
void copyFromShadowBuffer(const Rectangle& r)
{
    assert(g_videoWriteMode == 1);
    std::int16_t xOffsetBytes = r.x1 / 8;
    std::int16_t widthBytes = (r.x2 / 8) - xOffsetBytes + 1;
    std::int16_t height = r.y2 - r.y1;
    std::int16_t offset = xOffsetBytes + r.y1 * VIDEO_MEM_ROW_BYTES;

    VideoMemPtr dstPtr = offset;
    VideoMemPtr srcPtr = VIDEO_MEM_SHADOW_BUFFER + offset;
    for (std::uint16_t y = 0; y < height; ++y) {
        for (std::uint16_t x = 0; x < widthBytes; ++x)
            writeVideoMem(dstPtr++, readVideoMem(srcPtr++));
        dstPtr += VIDEO_MEM_ROW_BYTES - widthBytes;
        srcPtr += VIDEO_MEM_ROW_BYTES - widthBytes;
    }
}

/* 1b06:0062 */
VideoMemPtr copySpriteToShadowBuffer(VideoMemPtr dstPtr, std::int16_t x, std::int16_t y,
                                     std::int16_t width, std::int16_t height)
{
    assert(g_videoWriteMode == 1);
    VideoMemPtr srcPtr = VIDEO_MEM_START_ADDR + y * VIDEO_MEM_ROW_BYTES + (x / 8);
    for (std::int16_t curY = 0; curY < height; ++curY) {
        for (std::int16_t curX = 0; curX < width; ++curX)
            writeVideoMem(dstPtr++, readVideoMem(srcPtr++));
        srcPtr += VIDEO_MEM_ROW_BYTES - width;
    }
    return dstPtr;
}

/* 1b06:00a0 */
VideoMemPtr copySpriteFromShadowBuffer(VideoMemPtr srcPtr, std::int16_t x, std::int16_t y,
                                       std::int16_t width, std::int16_t height)
{
    assert(g_videoWriteMode == 1);
    VideoMemPtr dstPtr = VIDEO_MEM_START_ADDR + y * VIDEO_MEM_ROW_BYTES + (x / 8);
    for (std::int16_t curY = 0; curY < height; ++curY) {
        for (std::int16_t curX = 0; curX < width; ++curX)
            writeVideoMem(dstPtr++, readVideoMem(srcPtr++));
        dstPtr += VIDEO_MEM_ROW_BYTES - width;
    }
    return srcPtr;
}

} // namespace resl::drawing
