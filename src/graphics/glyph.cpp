#include "glyph.h"

#include "color.h"
#include "vga.h"
#include <system/driver/driver.h>
#include <utility/sar.h>

#include <bit>

namespace resl {

/* 1d7d:267a */
std::uint8_t g_glyphHeight = 0x10;

/* 1b06:062b */
void drawGlyphAlignX8(const Glyph* glyph, std::int16_t x, std::int16_t y, Color color)
{
    vga::VideoMemPtr videoPtr =
        vga::VIDEO_MEM_START_ADDR + y * vga::VIDEO_MEM_ROW_BYTES + sar(x, 3);
    const std::uint8_t* glyphPtr = glyph->data;
    for (std::uint8_t row = 0; row < glyph->height; ++row) {
        vga::VideoMemPtr v = videoPtr;
        for (std::uint8_t col = 0; col < glyph->width; ++col) {
            Driver::instance().vga().setWriteMask(*glyphPtr);
            Driver::instance().vga().write(v, color);
            ++v;
            ++glyphPtr;
        }
        videoPtr += vga::VIDEO_MEM_ROW_BYTES;
    }
}

/* 1b06:067e */
void drawGlyph(const Glyph& glyph, std::int16_t x, std::int16_t y, Color color)
{
    vga::VideoMemPtr videoPtr =
        vga::VIDEO_MEM_START_ADDR + y * vga::VIDEO_MEM_ROW_BYTES + sar(x, 3);
    const std::uint8_t pixelOffsetInsideByte = x & 7;
    const std::uint8_t rightMask = 0xFF >> pixelOffsetInsideByte;
    const std::uint8_t* glyphPtr = glyph.data;
    for (std::uint8_t row = 0; row < glyph.height; ++row) {
        for (std::uint8_t col = 0; col < glyph.width; ++col) {
            if (*glyphPtr) {
                std::uint8_t data = std::rotr(*glyphPtr, pixelOffsetInsideByte);
                Driver::instance().vga().setWriteMask(data & rightMask);
                Driver::instance().vga().write(videoPtr, color);

                Driver::instance().vga().setWriteMask(data & ~rightMask);
                Driver::instance().vga().write(videoPtr + 1, color);
            }
            ++videoPtr;
            ++glyphPtr;
        }
        videoPtr += vga::VIDEO_MEM_ROW_BYTES - glyph.width;
    }
}

/* 1b06:05cb */
void drawGlyphW8(const std::uint8_t* glyph, std::int16_t x, std::int16_t y, Color color)
{
    vga::VideoMemPtr videoPtr =
        vga::VIDEO_MEM_START_ADDR + y * vga::VIDEO_MEM_ROW_BYTES + sar(x, 3);
    const std::uint8_t pixelOffsetInsideByte = x & 7;
    const std::uint8_t rightMask = 0xFF >> pixelOffsetInsideByte;
    for (std::uint8_t row = 0; row < g_glyphHeight; ++row) {
        std::uint8_t data = std::rotr(*glyph++, pixelOffsetInsideByte);
        Driver::instance().vga().setWriteMask(data & rightMask);
        Driver::instance().vga().write(videoPtr, color);

        Driver::instance().vga().setWriteMask(data & ~rightMask);
        Driver::instance().vga().write(videoPtr + 1, color);

        videoPtr += vga::VIDEO_MEM_ROW_BYTES;
    }
}

/* 1b06:0548 */
void drawGlyphW16(const std::uint8_t* glyph, std::int16_t x, std::int16_t y, Color color)
{
    vga::VideoMemPtr videoPtr =
        vga::VIDEO_MEM_START_ADDR + y * vga::VIDEO_MEM_ROW_BYTES + sar(x, 3);
    const std::uint8_t pixelOffsetInsideByte = x & 7;
    const std::uint8_t rightMask = 0xFF >> pixelOffsetInsideByte;
    for (std::uint8_t row = 0; row < g_glyphHeight; ++row) {
        std::uint8_t data = std::rotr(*(glyph + 1), pixelOffsetInsideByte);

        Driver::instance().vga().setWriteMask(data & rightMask);
        Driver::instance().vga().write(videoPtr, color);

        Driver::instance().vga().setWriteMask(data & ~rightMask);
        Driver::instance().vga().write(videoPtr + 1, color);

        data = std::rotr(*glyph, pixelOffsetInsideByte);
        glyph += 2;
        Driver::instance().vga().setWriteMask(data & rightMask);
        Driver::instance().vga().write(videoPtr + 1, color);

        Driver::instance().vga().setWriteMask(data & ~rightMask);
        Driver::instance().vga().write(videoPtr + 2, color);

        videoPtr += vga::VIDEO_MEM_ROW_BYTES;
    }
}

} // namespace resl
