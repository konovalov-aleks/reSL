#include "drawing.h"

#include "draw_header.h"
#include "draw_impasse.h"
#include "game_data.h"
#include "graphics/drawing.h"
#include "resources/rail_glyph.h"
#include "resources/static_object_glyph.h"
#include <system/buffer.h>
#include <system/random.h>
#include <system/read_file.h>

#include <cassert>
#include <cstdint>
#include <cstdlib>
#include <initializer_list>

#include <algorithm> // std::min
#include <graphics/text.h>

namespace resl {

/* 137c:006f */
void drawRailBg1(
    std::int16_t tileX, std::int16_t tileY, std::int16_t railType, Color color, std::int16_t yOffset
)
{
    assert(railType >= 0 && railType <= std::size(railBackgrounds));
    RailGlyph* rg = railBackgrounds[railType].bg1;
    drawGlyph(
        &rg->glyph, (tileX - tileY) * 88 + rg->dx + 320,
        (tileX + tileY) * 21 + rg->dy + yOffset - 22, color
    );
}

/* 137c:00d2 */
void drawRailBg2(
    std::int16_t tileX, std::int16_t tileY, std::int16_t railType, Color color, std::int16_t yOffset
)
{
    assert(railType >= 0 && railType <= std::size(railBackgrounds));
    RailGlyph* rg = railBackgrounds[railType].bg2;
    drawGlyph(
        &rg->glyph, (tileX - tileY) * 88 + rg->dx + 320,
        (tileX + tileY) * 21 + rg->dy + yOffset - 22, color
    );
}

/* 137c:000c */
void drawRail(
    std::int16_t tileX, std::int16_t tileY, std::int16_t railType, Color color, std::int16_t yOffset
)
{
    assert(railType >= 0 && railType <= std::size(railBackgrounds));
    RailGlyph* rg = railBackgrounds[railType].mainGlyph;
    drawGlyph(
        &rg->glyph, (tileX - tileY) * 88 + rg->dx + 320,
        (tileX + tileY) * 21 + rg->dy + yOffset - 22, color
    );
}

/* 19de:0841 */
void scheduleAllTrainsRedrawing()
{
    for (std::int16_t i = 0; i < sizeof(trains) / sizeof(*trains); ++i)
        trains[i].needToRedraw = true;
}

/* 13d1:010f */
void drawSwitch(std::int16_t idx, bool x_someFlag)
{
    // TODO implement properly!!!
    Switch& s = g_switches[idx];
    Chunk* rail = s.curChunk.chunk;
    const RailGlyph* rg = railBackgrounds[rail->type].switches[s.curChunk.slot];
    (void)x_someFlag;
    // if (x_someFlag)
    drawGlyph(&rg->glyph, rail->x + rg->dx, rail->y + rg->dy, Color::Black);
    // drawGlyph(&rg->glyph, rail->x, rail->y + 350, Color::Black);
}

/* 13d1:026c */
void drawSwitch2(std::int16_t idx, std::int16_t yOffset)
{
    const Switch& s = g_switches[idx];
    const Chunk* rail = s.curChunk.chunk;
    const RailGlyph* rg = railBackgrounds[rail->type].switches[s.curChunk.slot];
    drawGlyph(&rg->glyph, rail->x + rg->dx, rail->y + rg->dy, Color::Black);
}

/* 137c:0135 */
void drawSemaphore(Semaphore& s, std::int16_t yOffset)
{
    const SemaphoreGlyph& glyph = *s.glyph;
    std::int16_t x = s.pixelX + glyph.xOffset;
    std::int16_t y = s.pixelY + glyph.yOffset + yOffset;

    g_glyphHeight = 15;
    drawGlyphW8(glyph.glyphBg1, x, y, Color::White);
    drawGlyphW8(glyph.glyphBg2, x, y, Color::Black);

    g_glyphHeight = 4;
    drawGlyphW16(
        s.glyph->glyphLight, x + glyph.lightXOffset, y + glyph.lightYOffset,
        s.isRed ? Color::Red : Color::LightGreen
    );
}

/* 1530:0203 */
inline bool isInsideGameField(int x, int y)
{
    return y >= 47 && y <= 333 && x >= 0 && x <= 639;
}

/* 1530:013e */
static void drawGrass(std::int16_t yOffset)
{
    const int seed = std::rand();
    std::srand(g_staticObjects[119].x);
    for (int i = 0; i < 25; ++i) {
        int x = genRandomNumber(640);
        int y = genRandomNumber(287) + 47;
        for (int j = 0; j < 45; ++j) {
            if (isInsideGameField(x, y)) {
                Color c = drawing::getPixel(x, y + yOffset);
                if (c == Color::Green)
                    drawing::putPixel(x, y + yOffset, Color::Black);
            }
            x += symmetricRand(20);
            y += symmetricRand(20) / 4;
        }
    }
    std::srand(seed);
}

/* 1530:0229 */
void drawStaticObjects(std::int16_t yOffset)
{
    drawGrass(yOffset);

    g_glyphHeight = 16;
    for (StaticObject& obj : g_staticObjects) {
        if (obj.kind == StaticObjectKind::House) {
            g_glyphHeight = std::min(334 - obj.y, 16);
            drawGlyphW16(g_houseGlyphs[obj.type].bg, obj.x, obj.y + yOffset, obj.color);
            drawGlyphW16(g_houseGlyphs[obj.type].fg, obj.x, obj.y + yOffset, Color::Black);
        } else if (obj.kind == StaticObjectKind::Tree) {
            g_glyphHeight = 16;
            drawGlyphW16(g_treeGlyphs[obj.type].bg, obj.x, obj.y + yOffset, obj.color);
            drawGlyphW16(g_treeGlyphs[obj.type].fg, obj.x, obj.y + yOffset, Color::Black);
        }
    }
}

/* 17bf:0cd0 */
void drawRailroad(std::int16_t yOffset)
{
    for (const RailInfo* r = g_railRoad; r < g_railRoad + g_railRoadCount; ++r) {
        drawRailBg1(r->tileX, r->tileY, r->railType, DarkGray, yOffset);
        drawRailBg2(r->tileX, r->tileY, r->railType, Gray, yOffset);
    }
    for (const RailInfo* r = g_railRoad; r < g_railRoad + g_railRoadCount; ++r) {
        drawRail(r->tileX, r->tileY, r->railType, Black, yOffset);
        drawImpasse(g_chunks[r->tileX][r->tileY][r->railType], yOffset);
    }

    for (std::uint16_t i = 0; i < g_nSwitches; ++i)
        drawSwitch(i, false);

    drawStaticObjects(yOffset);

    for (std::uint16_t i = 0; i < g_semaphoreCount; ++i)
        drawSemaphore(g_semaphores[i], yOffset);

    scheduleAllTrainsRedrawing();
    drawHeaderFieldFontTexture();
}

/* 132d:004b */
void fillGameFieldBackground(std::int16_t yOffset)
{
    drawing::filledRectangle(0, 49 + yOffset, 80, 285, 0xFF, Color::Green);
}

/* 132d:0086 */
void drawHeaderBackground(std::int16_t yOffset)
{
    readIfNotLoaded("play.7", g_pageBuffer);
    drawing::imageDot7(0, yOffset, 640, 47, g_pageBuffer);
}

/* 15e8:09c8 */
void drawWorld()
{
    fillGameFieldBackground(350);

    for (const RailInfo* r = g_railRoad; r < g_railRoad + g_railRoadCount; ++r) {
        drawRailBg1(r->tileX, r->tileY, r->railType, DarkGray, 350);
        drawRailBg2(r->tileX, r->tileY, r->railType, Gray, 350);
    }
    for (const RailInfo* r = g_railRoad; r < g_railRoad + g_railRoadCount; ++r) {
        drawRail(r->tileX, r->tileY, r->railType, Black, 350);
        drawImpasse(g_chunks[r->tileX][r->tileY][r->railType], 350);
    }

    for (std::uint16_t i = 0; i < g_nSwitches; ++i)
        drawSwitch2(i, 350);

    drawStaticObjects(350);

    for (std::uint16_t i = 0; i < g_semaphoreCount; ++i) {
        const Semaphore& s = g_semaphores[i];
        if (!s.isRightDirection)
            drawSemaphore(g_semaphores[i], 350);
    }

    // TODO
    // x_schedule_trains_drawing();
    // x_drawTrains()

    for (std::uint16_t i = 0; i < g_semaphoreCount; ++i) {
        const Semaphore& s = g_semaphores[i];
        if (s.isRightDirection)
            drawSemaphore(g_semaphores[i], 350);
    }

    drawHeaderBackground(0x15e);
    // TODO
    //     drawHeaderData(headers.trains.value,headers.money.value,headers.year.value,headers.level.value,
    //                    0x15e);
    //     drawAllDispatchers(0x15e);
    //     drawFooterWithCopyright(0x15e);
}

} // namespace resl
