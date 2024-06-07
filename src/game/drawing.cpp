#include "drawing.h"

#include "draw_header.h"
#include "draw_impasse.h"
#include "game_data.h"
#include "graphics/drawing.h"
#include "header.h"
#include "resources/rail_glyph.h"
#include "resources/static_object_glyph.h"
#include "resources/train_finished_exclamation_glyph.h"
#include "semaphore.h"
#include "status_bar.h"
#include "switch.h"
#include "train.h"
#include "types/chunk.h"
#include "types/header_field.h"
#include "types/rail_info.h"
#include "types/static_object.h"
#include <graphics/color.h>
#include <graphics/glyph.h>
#include <system/buffer.h>
#include <system/random.h>
#include <system/read_file.h>

#include <algorithm>
#include <cassert>
#include <cstdint>
#include <cstdlib>

#ifndef NDEBUG
#   include <iterator>
#endif

namespace resl {

/* 137c:006f */
void drawRailBg1(std::int16_t tileX, std::int16_t tileY,
                 std::int16_t railType, Color color, std::int16_t yOffset)
{
    assert(railType >= 0 && railType <= std::size(railBackgrounds));
    RailGlyph* rg = railBackgrounds[railType].bg1;
    drawGlyphAlignX8(&rg->glyph, (tileX - tileY) * 88 + rg->dx + 320,
                     (tileX + tileY) * 21 + rg->dy + yOffset - 22, color);
}

/* 137c:00d2 */
void drawRailBg2(std::int16_t tileX, std::int16_t tileY,
                 std::int16_t railType, Color color, std::int16_t yOffset)
{
    assert(railType >= 0 && railType <= std::size(railBackgrounds));
    RailGlyph* rg = railBackgrounds[railType].bg2;
    drawGlyphAlignX8(&rg->glyph, (tileX - tileY) * 88 + rg->dx + 320,
                     (tileX + tileY) * 21 + rg->dy + yOffset - 22, color);
}

/* 137c:000c */
void drawRail(std::int16_t tileX, std::int16_t tileY,
              std::int16_t railType, Color color, std::int16_t yOffset)
{
    assert(railType >= 0 && railType <= std::size(railBackgrounds));
    RailGlyph* rg = railBackgrounds[railType].mainGlyph;
    drawGlyphAlignX8(&rg->glyph, (tileX - tileY) * 88 + rg->dx + 320,
                     (tileX + tileY) * 21 + rg->dy + yOffset - 22, color);
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
void drawFieldBackground(std::int16_t yOffset)
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

    scheduleTrainsDrawing();
    drawTrains();

    for (std::uint16_t i = 0; i < g_semaphoreCount; ++i) {
        const Semaphore& s = g_semaphores[i];
        if (s.isRightDirection)
            drawSemaphore(g_semaphores[i], 350);
    }

    drawHeaderBackground(350);
    drawHeaderData(g_headers[static_cast<std::size_t>(HeaderFieldId::Trains)].value,
                   g_headers[static_cast<std::size_t>(HeaderFieldId::Money)].value,
                   g_headers[static_cast<std::size_t>(HeaderFieldId::Year)].value,
                   g_headers[static_cast<std::size_t>(HeaderFieldId::Level)].value, 350);
    drawDispatchers(350);

    drawStatusBarWithCopyright(350);
}

/* 132d:01e2 */
void drawEraseTrainFinishedExclamation(std::int16_t entranceX, std::int16_t entranceY)
{
    // Uses XOR operation when drawing
    // => this allows us to use the same code to draw and erase the image

    std::int16_t x;
    const Glyph* glyph;
    if (entranceX < 320) {
        x = 8;
        glyph = &g_glyphTrainFinishedLeftEntrance;
    } else {
        x = 611;
        glyph = &g_glyphTrainFinishedRightEntrance;
    }

    drawing::setDataRotation(0x18); // rotation = 0, mode = XOR
    drawGlyphAlignX8(glyph, x, entranceY - 9, Color::White);
    drawing::setDataRotation(0); // default mode
}

/* 132d:00f9 */
void drawGameField(std::int16_t yOffset)
{
    // The file name in the original game is lowercase:
    //  1d7d:01a0 "play.7"
    // But DOS filesystem is case-insensitive => the fact that the file name on disk
    // and name in the code are in different case is not a problem there.
    // For portability, I use a name identical to the file name on disk.
    readIfNotLoaded("PLAY.7", g_pageBuffer);
    drawing::imageDot7(0, yOffset, 640, 350, g_pageBuffer);
    drawStaticObjects(yOffset);
    drawCopyright(yOffset);
}

} // namespace resl
