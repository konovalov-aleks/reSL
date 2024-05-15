#include "glyph.h"

#include "driver.h"

namespace resl {

/* 1d7d:267a */
std::uint8_t g_glyphHeight = 0x10;

inline std::uint8_t ror8(std::uint8_t x, std::uint8_t count)
{
    return (x >> count) | (x << (8 - count));
}

/* 1b06:062b */
void drawGlyphAlignX8(const Glyph* glyph, std::int16_t x, std::int16_t y, Color color)
{
    unsigned videoPtr = VIDEO_MEM_START_ADDR + y * VIDEO_MEM_ROW_BYTES + (x >> 3);
    const std::uint8_t* glyphPtr = glyph->data;
    for (std::uint8_t y = 0; y < glyph->height; ++y) {
        unsigned v = videoPtr;
        for (std::uint8_t x = 0; x < glyph->width; ++x) {
            setVideoMask(*glyphPtr);
            writeVideoMem(v, color);
            ++v;
            ++glyphPtr;
        }
        videoPtr += VIDEO_MEM_ROW_BYTES;
    }
}

/* 1b06:067e */
void drawGlyph(const Glyph* glyph, std::int16_t x, std::int16_t y, Color color)
{
    unsigned videoPtr = VIDEO_MEM_START_ADDR + y * VIDEO_MEM_ROW_BYTES + (x >> 3);
    const std::uint8_t pixelOffsetInsideByte = x & 7;
    const std::uint8_t rightMask = 0xFF >> pixelOffsetInsideByte;
    const std::uint8_t* glyphPtr = glyph->data;
    for (std::uint8_t y = 0; y < glyph->height; ++y) {
        for (std::uint8_t x = 0; x < glyph->width; ++x) {
            if (*glyphPtr) {
                std::uint8_t data = ror8(*glyphPtr, pixelOffsetInsideByte);
                setVideoMask(data & rightMask);
                writeVideoMem(videoPtr, color);

                setVideoMask(data & ~rightMask);
                writeVideoMem(videoPtr + 1, color);
            }
            ++videoPtr;
            ++glyphPtr;
        }
        videoPtr += VIDEO_MEM_ROW_BYTES - glyph->width;
    }
}

/* 1b06:05cb */
void drawGlyphW8(const std::uint8_t* glyph, std::int16_t x, std::int16_t y, Color color)
{
    unsigned glyphPtr = 0;
    unsigned videoPtr = VIDEO_MEM_START_ADDR + y * VIDEO_MEM_ROW_BYTES + (x >> 3);
    const std::uint8_t pixelOffsetInsideByte = x & 7;
    const std::uint8_t rightMask = 0xFF >> pixelOffsetInsideByte;
    for (int i = 0; i < g_glyphHeight; ++i) {
        std::uint8_t data = ror8(glyph[glyphPtr + 1], pixelOffsetInsideByte);
        setVideoMask(data & rightMask);
        writeVideoMem(videoPtr, color);

        setVideoMask(data & ~rightMask);
        writeVideoMem(videoPtr + 1, color);

        videoPtr += VIDEO_MEM_ROW_BYTES;
        ++glyphPtr;
    }
}

/* 1b06:0548 */
void drawGlyphW16(const std::uint8_t* glyph, int16_t x, int16_t y, Color color)
{
    unsigned glyphPtr = 0;
    unsigned videoPtr = VIDEO_MEM_START_ADDR + y * VIDEO_MEM_ROW_BYTES + (x >> 3);
    const std::uint8_t pixelOffsetInsideByte = x & 7;
    const std::uint8_t rightMask = 0xff >> pixelOffsetInsideByte;
    for (int i = 0; i < g_glyphHeight; ++i) {
        std::uint8_t data = ror8(glyph[glyphPtr + 1], pixelOffsetInsideByte);

        setVideoMask(data & rightMask);
        writeVideoMem(videoPtr, color);

        setVideoMask(data & ~rightMask);
        writeVideoMem(videoPtr + 1, color);

        data = ror8(glyph[glyphPtr], pixelOffsetInsideByte);
        glyphPtr += 2;
        setVideoMask(data & rightMask);
        writeVideoMem(videoPtr + 1, color);

        setVideoMask(data & ~rightMask);
        writeVideoMem(videoPtr + 2, color);

        videoPtr += VIDEO_MEM_ROW_BYTES;
    }
}

} // namespace resl
