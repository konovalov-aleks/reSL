#include "impasse.h"

#include "rail.h"
#include "resources/glyph_empty_background.h"
#include "resources/impasse_glyph.h"
#include "resources/rail_connection_bias.h"
#include "resources/semaphore_glyph_bias.h"
#include <graphics/color.h>
#include <graphics/glyph.h>

#include <cstdint>

namespace resl {

/* 137c:024d */
void drawImpasse(const Rail& r, std::int16_t yOffset)
{
    for (int i = 0; i < 2; ++i) {
        if (r.connections[i].rail)
            continue;

        const SemaphoreGlyphBias& gb = g_semaphoreGlyphBiases[r.type][i];
        const RailConnectionBias& rc = g_railConnectionBiases[r.type][i];
        const std::int16_t x =
            r.x + (rc.tileOffsetX - rc.tileOffsetY) * 88 + (gb.dx * 2) / 3 - 8;
        const std::int16_t y =
            r.y + (rc.tileOffsetX + rc.tileOffsetY) * 21 + (gb.dy * 2) / 3 + yOffset - 5;

        const int direction = (gb.dx ^ gb.dy) > 0;

        g_glyphHeight = 8;
        drawGlyphW16(g_impasseGlyphs[direction].bg, x, y, Color::Black);
        g_glyphHeight = 8;
        drawGlyphW16(g_impasseGlyphs[direction].fg, x, y, Color::White);
    }
}

/* 137c:0378 */
void eraseImpasse(const Rail& r, std::int16_t yOffset)
{
    for (int i = 0; i < 2; ++i) {
        const SemaphoreGlyphBias& gb = g_semaphoreGlyphBiases[r.type][i];
        const RailConnectionBias& rc = g_railConnectionBiases[r.type][i];

        g_glyphHeight = 8;
        drawGlyphW16(
            g_glyphEmptyBackground,
            r.x + (rc.tileOffsetX - rc.tileOffsetY) * 88 - (gb.dx * 2) / 3 - 8,
            r.y + (rc.tileOffsetX + rc.tileOffsetY) * 21 - (gb.dy * 2) / 3 + yOffset - 5,
            Color::Green);
    }
}

} // namespace resl
