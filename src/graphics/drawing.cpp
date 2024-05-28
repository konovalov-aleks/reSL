#include "drawing.h"

#include "color.h"
#include "vga.h"
#include <game/types/rectangle.h>
#include <system/driver/driver.h>
#include <utility/ror.h>
#include <utility/sar.h>

#include <cassert>
#include <cstdlib>
#include <utility>

namespace resl::drawing {

/* 1b06:026d */
static void setVideoModeR0W0()
{
    Driver::instance().vga().setMode(0);
    Driver::instance().vga().setWriteMask(0xFF);
}

/* 1b06:0279 */
void setVideoModeR0W1()
{
    Driver::instance().vga().setMode(1);
    Driver::instance().vga().setWriteMask(0xFF);
}

/* 1b06:0285 */
void setVideoModeR0W2()
{
    Driver::instance().vga().setMode(2);
    Driver::instance().vga().setWriteMask(0xFF);
}

/* 1b06:02a5 */
void setDataRotation(std::uint8_t rotation)
{
    assert((rotation & 7) == 0); // not implemented yet
    rotation >>= 3;
    assert(rotation < 4);
    Driver::instance().vga().setWriteOperation(static_cast<WriteOperation>(rotation));
}

/* 1b06:02b5 */
static void videoChoosePlanes(std::uint8_t mask)
{
    Driver::instance().vga().setMapMask(mask);
}

/* 1b06:06f9 */
void filledRectangle(std::int16_t x, std::int16_t y,
                     std::int16_t width, std::int16_t height,
                     std::uint8_t pattern, Color color)
{
    assert(Driver::instance().vga().writeMode() == 2);
    VideoMemPtr videoPtr = VIDEO_MEM_START_ADDR + y * VIDEO_MEM_ROW_BYTES + sar(x, 3);
    Driver::instance().vga().setWriteMask(pattern);
    for (int curY = 0; curY < height; ++curY) {
        VideoMemPtr vp = videoPtr;
        for (std::int16_t curX = 0; curX < width; ++curX)
            Driver::instance().vga().write(vp++, color);
        videoPtr += VIDEO_MEM_ROW_BYTES;
    }
}

/* 1b06:0956 */
void horizontalLine(std::int16_t x1, std::int16_t x2, std::int16_t y, Color color)
{
    if (x2 < x1)
        std::swap(x1, x2);

    VideoMemPtr videoPtr1 = VIDEO_MEM_START_ADDR + y * VIDEO_MEM_ROW_BYTES + sar(x1, 3);
    VideoMemPtr videoPtr2 = VIDEO_MEM_START_ADDR + y * VIDEO_MEM_ROW_BYTES + sar(x2, 3);

    const std::uint8_t pixelOffsetInsideByte1 = x1 & 7;
    const std::uint8_t pixelOffsetInsideByte2 = x2 & 7;

    if (videoPtr1 == videoPtr2) {
        // single point
        Driver::instance().vga().setWriteMask(
            (ror<std::uint16_t>(0x80FF, pixelOffsetInsideByte2) >> 8) & (0xFF >> pixelOffsetInsideByte1) & 0xFF);
        Driver::instance().vga().write(videoPtr1, color);
    } else {
        Driver::instance().vga().setWriteMask(0xFF >> pixelOffsetInsideByte1);
        Driver::instance().vga().write(videoPtr1++, color);

        Driver::instance().vga().setWriteMask(0xFF);
        for (std::int16_t i = 0, cnt = videoPtr2 - videoPtr1; i < cnt; ++i)
            Driver::instance().vga().write(videoPtr1++, color);

        Driver::instance().vga().setWriteMask((ror<std::uint16_t>(0x80FF, pixelOffsetInsideByte2) >> 8) & 0xFF);
        Driver::instance().vga().write(videoPtr1++, color);
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
    VideoMemPtr videoPtr = VIDEO_MEM_START_ADDR + y * VIDEO_MEM_ROW_BYTES + sar(x, 3);
    const int nRows = 7;
    if (drawSprite) {
        // copy sprite data
        for (int r = 0; r < nRows; ++r) {
            Driver::instance().vga().write(videoPtr, *(data++));
            videoPtr += VIDEO_MEM_ROW_BYTES;
        }
    } else {
        // fill with a constant
        const std::uint8_t v = *data;
        for (int r = 0; r < nRows; ++r) {
            Driver::instance().vga().write(videoPtr, v);
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
    const VideoMemPtr videoPtr = VIDEO_MEM_START_ADDR + y * VIDEO_MEM_ROW_BYTES + sar(x, 3);
    const std::uint8_t mask = 0x80 >> (x & 7);

    Driver::instance().vga().setReadPlane(0);
    std::uint8_t res = (Driver::instance().vga().read(videoPtr) & mask) != 0;

    Driver::instance().vga().setReadPlane(1);
    if (Driver::instance().vga().read(videoPtr) & mask)
        res |= 2;

    Driver::instance().vga().setReadPlane(2);
    if (Driver::instance().vga().read(videoPtr) & mask)
        res |= 4;

    Driver::instance().vga().setReadPlane(3);
    if (Driver::instance().vga().read(videoPtr) & mask)
        res |= 8;

    return static_cast<Color>(res);
}

/* 1b06:0838 */
void putPixel(std::int16_t x, std::int16_t y, Color c)
{
    const VideoMemPtr videoPtr = VIDEO_MEM_START_ADDR + y * VIDEO_MEM_ROW_BYTES + sar(x, 3);

    Driver::instance().vga().setWriteMask(0x80 >> (x & 7));
    Driver::instance().vga().write(videoPtr, c);

    assert(getPixel(x, y) == c);
}

/* 1b06:086a */
void line(std::int16_t x1, std::int16_t y1, std::int16_t x2, std::int16_t y2, Color c)
{
    // https://en.wikipedia.org/wiki/Bresenham%27s_line_algorithm#All_cases

    if (x2 < x1) {
        std::swap(x1, x2);
        std::swap(y1, y2);
    }
    std::int16_t dx = x2 - x1;
    std::int16_t dy = y2 - y1;

    const bool negativeDY = y2 < y1;
    if (negativeDY)
        dy = -dy;

    const bool inverseAxis = dx > dy;
    if (inverseAxis)
        std::swap(dx, dy);

    if (dy == 0)
        return;

    const int mode = (negativeDY << 2) | (inverseAxis << 1);

    std::int16_t err = 2 * dx - dy;

    VideoMemPtr videoPtr = VIDEO_MEM_START_ADDR + y1 * VIDEO_MEM_ROW_BYTES + sar(x1, 3);
    std::uint8_t pixelMask = 0x80 >> (x1 & 7);

    for (std::int16_t y = 0; y < dy; ++y) {
        Driver::instance().vga().setWriteMask(pixelMask);
        Driver::instance().vga().write(videoPtr, c);

        const auto moveAlongX = [&videoPtr, &pixelMask]() {
            std::uint8_t lastBit = pixelMask & 1;
            pixelMask = ror(pixelMask, 1);
            videoPtr += lastBit;
        };

        bool isNegative = err < 0;
        switch (mode | isNegative) {
        case 0:
            moveAlongX();
            [[fallthrough]];
        case 1:
            videoPtr += VIDEO_MEM_ROW_BYTES;
            break;
        case 2:
            videoPtr += VIDEO_MEM_ROW_BYTES;
            [[fallthrough]];
        case 3:
            moveAlongX();
            break;
        case 4:
            moveAlongX();
            [[fallthrough]];
        case 5:
            videoPtr -= VIDEO_MEM_ROW_BYTES;
            break;
        case 6:
            videoPtr -= VIDEO_MEM_ROW_BYTES;
            [[fallthrough]];
        case 7:
            moveAlongX();
            break;
        default:
            std::abort(); // unreachable
        }
        if (!isNegative)
            err -= 2 * dy;
        err += 2 * dx;
    }
}

/* 1b06:0324 */
void copyRectangle(std::int16_t dstX, std::int16_t dstY,
                   std::int16_t srcX, std::int16_t srcY,
                   std::int16_t width, std::int16_t height)
{
    Driver::instance().vga().setMode(1);

    VideoMemPtr srcVideoPtr = VIDEO_MEM_START_ADDR + srcY * VIDEO_MEM_ROW_BYTES + sar(srcX, 3);
    VideoMemPtr dstVideoPtr = VIDEO_MEM_START_ADDR + dstY * VIDEO_MEM_ROW_BYTES + sar(dstX, 3);

    for (std::int16_t y = 0; y < height; ++y) {
        for (std::int16_t x = 0; x < width; ++x) {
            std::uint8_t data = Driver::instance().vga().read(srcVideoPtr++);
            Driver::instance().vga().write(dstVideoPtr++, data);
        }
        srcVideoPtr += VIDEO_MEM_ROW_BYTES - width;
        dstVideoPtr += VIDEO_MEM_ROW_BYTES - width;
    }
}

/* 1b06:0004 */
void copyFromShadowBuffer(const Rectangle& r)
{
    assert(Driver::instance().vga().writeMode() == 1);
    std::int16_t xOffsetBytes = sar(r.x1, 3);
    std::int16_t widthBytes = sar(r.x2, 3) - xOffsetBytes + 1;
    std::int16_t height = r.y2 - r.y1;
    std::int16_t offset = xOffsetBytes + r.y1 * VIDEO_MEM_ROW_BYTES;

    VideoMemPtr dstPtr = VIDEO_MEM_START_ADDR + offset;
    VideoMemPtr srcPtr = VIDEO_MEM_SHADOW_BUFFER + offset;
    for (std::uint16_t y = 0; y < height; ++y) {
        for (std::uint16_t x = 0; x < widthBytes; ++x)
            Driver::instance().vga().write(dstPtr++, Driver::instance().vga().read(srcPtr++));
        dstPtr += VIDEO_MEM_ROW_BYTES - widthBytes;
        srcPtr += VIDEO_MEM_ROW_BYTES - widthBytes;
    }
}

/* 1b06:0062 */
VideoMemPtr copySpriteToShadowBuffer(VideoMemPtr dstPtr, std::int16_t x, std::int16_t y,
                                     std::int16_t width, std::int16_t height)
{
    assert(Driver::instance().vga().writeMode() == 1);
    VideoMemPtr srcPtr = VIDEO_MEM_START_ADDR + y * VIDEO_MEM_ROW_BYTES + sar(x, 3);
    for (std::int16_t curY = 0; curY < height; ++curY) {
        for (std::int16_t curX = 0; curX < width; ++curX)
            Driver::instance().vga().write(dstPtr++, Driver::instance().vga().read(srcPtr++));
        srcPtr += VIDEO_MEM_ROW_BYTES - width;
    }
    return dstPtr;
}

/* 1b06:00a0 */
VideoMemPtr copySpriteFromShadowBuffer(VideoMemPtr srcPtr, std::int16_t x, std::int16_t y,
                                       std::int16_t width, std::int16_t height)
{
    assert(Driver::instance().vga().writeMode() == 1);
    VideoMemPtr dstPtr = VIDEO_MEM_START_ADDR + y * VIDEO_MEM_ROW_BYTES + sar(x, 3);
    for (std::int16_t curY = 0; curY < height; ++curY) {
        for (std::int16_t curX = 0; curX < width; ++curX)
            Driver::instance().vga().write(dstPtr++, Driver::instance().vga().read(srcPtr++));
        dstPtr += VIDEO_MEM_ROW_BYTES - width;
    }
    return srcPtr;
}

} // namespace resl::drawing
