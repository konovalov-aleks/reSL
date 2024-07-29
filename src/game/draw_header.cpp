#include "draw_header.h"

#include "entrance.h"
#include "header.h"
#include "resources/dispatcher_glyph.h"
#include "resources/glyph_empty_background.h"
#include "types/header_field.h"
#include <graphics/color.h>
#include <graphics/drawing.h>
#include <graphics/glyph.h>
#include <graphics/text.h>
#include <graphics/vga.h>
#include <system/buffer.h>
#include <system/filesystem.h>

#include <cstdio>

namespace resl {

/* 1d7d:0138 : 48 bytes */
static const char* const s_alphabet[12] = { "9", "0", "1", "2", "3", "4",
                                            "5", "6", "7", "8", "9", "0" };

/* 12c5:0342 */
void drawHeaderFieldFontTexture()
{
    graphics::filledRectangle(680, 397, 2, 192, 0xFF, Color::White);
    graphics::filledRectangle(680, 397, 1, 192, 0x80, Color::DarkGray);
    for (int i = 0; i < 12; ++i)
        drawText(682, i * 16 + 399, s_alphabet[i], Color::Black);
}

/* 132d:0086 */
void drawHeaderBackground(std::int16_t yOffset)
{
    readIfNotLoaded("play.7", g_pageBuffer);
    graphics::imageDot7(0, yOffset, SCREEN_WIDTH, g_headerHeight, g_pageBuffer);
}

/* 12c5:01d7 */
void drawHeaderField(const HeaderField& hdrField)
{
    for (std::int8_t i = 0; i <= hdrField.curAnimatingDigit; ++i) {
        graphics::copyRectangle(
            hdrField.x + (hdrField.nDigits - i - 1) * 16,
            hdrField.y, 680,
            (hdrField.digitValues[i] + 1) * 16 + hdrField.yScroll + 397,
            2, 16);
    }
    vga::setVideoModeR0W2();
}

/* 12c5:022d */
static void updateHeaderField(
    std::int16_t fieldIdx, std::int16_t newValue, std::int16_t yOffset)
{
    HeaderField& hdrField = g_headers[fieldIdx];
    hdrField.value = newValue;

    char buf[10];
    std::snprintf(buf, sizeof(buf), "%5d", static_cast<int>(hdrField.value));
    for (int i = 0; i < 5; ++i) {
        if (buf[i] == ' ')
            buf[i] = '0';
    }

    for (int i = 0; i < hdrField.nDigits; ++i)
        hdrField.digitValues[i] = buf[4 - i] - '0';

    hdrField.yScroll = 0;
    hdrField.curAnimatingDigit = hdrField.nDigits - 1;
    hdrField.y += yOffset;
    drawHeaderField(hdrField);
    hdrField.y -= yOffset;
}

/* 12c5:02d1 */
void drawHeaderData(
    std::int16_t trains, std::int16_t money, std::int16_t year,
    std::int16_t level, std::int16_t yOffset)
{
    drawHeaderFieldFontTexture();
    updateHeaderField(0, trains, yOffset);
    updateHeaderField(1, money, yOffset);
    updateHeaderField(2, year, yOffset);
    updateHeaderField(3, level, yOffset);
}

/* 137c:0450 */
void drawDispatcher(std::int16_t entranceIdx, bool signalling)
{
    const std::int16_t x = entranceIdx * 22 + 481;
    g_glyphHeight = 16;
    drawGlyphW16(g_glyphEmptyBackground, x, 25, Color::Gray);
    drawGlyphW16(g_dispatcherGlyphs[signalling].bg, x, 25, g_entrances[entranceIdx].bgColor);
    drawGlyphW16(g_dispatcherGlyphs[signalling].fg, x, 25, Color::Black);
}

/* 137c:04c3 */
void drawDispatchers(std::int16_t yOffset)
{
    g_glyphHeight = 16;

    const std::int16_t y = yOffset + 25;
    for (std::int16_t i = 0; i < g_entranceCount; ++i) {
        const Entrance& e = g_entrances[i];
        bool signalling = e.waitingTrainsCount != 0;
        const std::int16_t x = i * 22 + 481;
        drawGlyphW16(g_dispatcherGlyphs[signalling].bg, x, y, e.bgColor);
        drawGlyphW16(g_dispatcherGlyphs[signalling].fg, x, y, Color::Black);
    }
}

} // namespace resl
