#include "draw_impasse.h"

#include "graphics/glyph.h"
#include "resources/impasse_glyph.h"
#include "resources/s4arr.h"
#include "resources/semaphore_glyph_bias.h"

#include <cstdint>

namespace resl {

/* 137c:024d */
void drawImpasse(const Chunk& c, std::int16_t yOffset)
{
    for (int i = 0; i < 2; ++i) {
        if (c.x_neighbours[i].chunk)
            continue;

        const SemaphoreGlyphBias& gb = g_semaphoreGlyphBias[c.type][i];
        const s4& reInfo = s4arr[c.type][i];
        const std::int16_t x =
            c.x + (reInfo.tileOffsetX - reInfo.tileOffsetY) * 88 + (gb.dx * 2) / 3 - 8;
        const std::int16_t y =
            c.y + (reInfo.tileOffsetX + reInfo.tileOffsetY) * 21 + (gb.dy * 2) / 3 + yOffset - 5;

        const int direction = (gb.dx ^ gb.dy) > 0;

        g_glyphHeight = 8;
        drawGlyphW16(g_impasseGlyphs[direction].bg, x, y, Color::Black);
        g_glyphHeight = 8;
        drawGlyphW16(g_impasseGlyphs[direction].fg, x, y, Color::White);
    }
}

} // namespace resl
