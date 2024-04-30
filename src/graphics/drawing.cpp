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
static void graphics_setWrite0Mode()
{
    setVideoWriteMode(0);
    setVideoMask(0xFF);
}

/* 1b06:0285 */
static void graphics_setWriteMode2()
{
    setVideoWriteMode(2);
    setVideoMask(0xFF);
}

/* 1b06:02b5 */
static void videoChoosePlanes(std::uint8_t mask)
{
    setVideoMapMask(mask);
}

/* 1b06:06f9 */
void filledRectangle(
    std::int16_t x, std::int16_t y, std::int16_t width, std::int16_t height, std::uint8_t pattern,
    Color color
)
{
    unsigned videoPtr = VIDEO_MEM_START_ADDR + y * VIDEO_MEM_ROW_BYTES + (x >> 3);
    setVideoMask(pattern);
    for (int curY = 0; curY < height; ++curY) {
        unsigned vp = videoPtr;
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

    unsigned videoPtr1 = VIDEO_MEM_START_ADDR + y * VIDEO_MEM_ROW_BYTES + (x1 >> 3);
    unsigned videoPtr2 = VIDEO_MEM_START_ADDR + y * VIDEO_MEM_ROW_BYTES + (x2 >> 3);

    const std::uint8_t pixelOffsetInsideByte1 = x1 & 7;
    const std::uint8_t pixelOffsetInsideByte2 = x2 & 7;

    if (videoPtr1 == videoPtr2) {
        // single point
        setVideoMask(
            (ror16(0x80FF, pixelOffsetInsideByte2) >> 8) & (0xFF >> pixelOffsetInsideByte1) & 0xFF
        );
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
void dialogFrame(
    std::int16_t x, std::int16_t y, std::int16_t width, std::int16_t height, Color bgColor
)
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
static void
implDrawImageDot7(std::int16_t x, std::int16_t y, const std::uint8_t* data, bool drawSprite)
{
    unsigned videoPtr = VIDEO_MEM_START_ADDR + y * VIDEO_MEM_ROW_BYTES + (x >> 3);
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
void imageDot7(
    std::int16_t x, std::int16_t y, std::int16_t width, std::int16_t height,
    const std::uint8_t* image
)
{
    const std::uint8_t* data = &image[0x10];
    graphics_setWrite0Mode();
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
    graphics_setWriteMode2();
    videoChoosePlanes(0xF);
}

/* 1b06:07db */
Color getPixel(std::int16_t x, std::int16_t y)
{
    const unsigned videoPtr = VIDEO_MEM_START_ADDR + y * VIDEO_MEM_ROW_BYTES + (x >> 3);
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
    const unsigned videoPtr = VIDEO_MEM_START_ADDR + y * VIDEO_MEM_ROW_BYTES + (x >> 3);

    setVideoMask(0x80 >> (x & 7));
    writeVideoMem(videoPtr, c);

    assert(getPixel(x, y) == c);
}

} // namespace resl::drawing
