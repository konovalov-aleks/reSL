#include "draw_header.h"

#include "game_data.h"
#include "resources/dispatcher_glyph.h"
#include <graphics/drawing.h>
#include <graphics/glyph.h>
#include <graphics/text.h>
#include <system/buffer.h>
#include <system/read_file.h>

#include <cstdio>

namespace resl {

/* 1d7d:0138 : 48 bytes */
static const char* const s_alphabet[12] = { "9", "0", "1", "2", "3", "4",
                                            "5", "6", "7", "8", "9", "0" };

/* 12c5:0342 */
void drawHeaderFieldFontTexture()
{
    drawing::filledRectangle(680, 397, 2, 192, 0xFF, Color::White);
    drawing::filledRectangle(680, 397, 1, 192, 0x80, Color::DarkGray);
    for (int i = 0; i < 12; ++i)
        drawText(682, i * 16 + 399, s_alphabet[i], Color::Black);
}

/* 132d:0086 */
void drawHeaderBackground(std::int16_t yOffset)
{
    readIfNotLoaded("play.7", g_pageBuffer);
    drawing::imageDot7(0, yOffset, 640, 47, g_pageBuffer);
}

/* 12c5:01d7 */
static void drawHeaderField(const HeaderField& hdrField)
{
    for (int i = 0; i < hdrField.nDigits; ++i) {
        drawing::copyRectangle(
            hdrField.x + (hdrField.nDigits - i - 1) * 16,
            hdrField.y, 680,
            (hdrField.digitValues[i] + 1) * 16 + hdrField._counter + 397,
            2, 16);
    }
    drawing::setVideoModeR0W2();
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

    hdrField._counter = 0;
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

/* 137c:04c3 */
void drawDispatchers(std::int16_t yOffset)
{
    g_glyphHeight = 16;

    const std::int16_t y = yOffset + 25;
    for (int i = 0; i < entranceCount; ++i) {
        const EntranceInfo& e = g_entrances[i];
        bool signalling = e.waitingTrainsCount != 0;
        const std::int16_t x = i * 22 + 481;
        drawGlyphW16(g_dispatcherGlyphs[signalling].bg, x, y, e.bgColor);
        drawGlyphW16(g_dispatcherGlyphs[signalling].fg, x, y, Color::Black);
    }
}

} // namespace resl
