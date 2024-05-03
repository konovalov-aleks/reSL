#include "drawing.h"

#include "draw_header.h"
#include "draw_impasse.h"
#include "game_data.h"
#include "graphics/drawing.h"
#include "resources/carriage_bias.h"
#include "resources/movement_paths.h"
#include "resources/rail_glyph.h"
#include "resources/static_object_glyph.h"
#include "resources/train_glyph.h"
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

/* 18fa:0142 */
static bool trainOverlaps(const Train& t, int idx)
{
    for (int i = t.carriageCnt - 1; i >= 0; --i) {
        const Carriage& c1 = t.carriages[i];
        for (const Carriage* c2 = g_trainDrawingChains[idx]; c2; c2 = c2->next) {
            // Check if the rectangles overlap
            if (c1.rect.y1 < c2->rect.y2 && c1.rect.y2 > c2->rect.y1 &&
                (c1.rect.x1 & ~7) <= (c2->rect.x2 & ~7) &&
                (c1.rect.x2 & ~7) >= (c2->rect.x1 & ~7)) {

                x_orderArray[g_orderArrayLen++] = { &c1, c2 };
                return true;
            }
        }
    }
    return false;
}

/* 18fa:00e9 */
static void addToDrawingChain(Train& t, int chainIdx)
{
    for (int i = t.carriageCnt - 1; i >= 0; --i) {
        Carriage& c1 = t.carriages[i];
        Carriage** c2 = &g_trainDrawingChains[chainIdx];
        while (*c2 && (*c2)->drawingPriority < c1.drawingPriority)
            c2 = &(*c2)->next;
        c1.next = *c2;
        *c2 = &c1;
    }
    t.drawingChainIdx = chainIdx;
}

/* 18fa:01e1 */
static void mergeDrawingChains(int idx1, int idx2)
{
    Carriage* c1 = g_trainDrawingChains[idx1];
    Carriage* c2 = g_trainDrawingChains[idx2];
    g_trainDrawingChains[idx2] = nullptr;
    while (c2) {
        c2->train->drawingChainIdx = idx1;
        while (c1->next->drawingPriority < c2->drawingPriority) {
            c1 = c1->next;
            if (!c1->next) {
                c1->next = c2;
                return;
            }
        }
        Carriage* tmp = c2->next;
        c2->next = c1->next;
        c1->next = c2;
        c1 = c1->next;
        c2 = tmp;
    }
}

/* 18fa:000b */
static void scheduleTrainsDrawing()
{
    g_orderArrayLen = 0;
    g_trainDrawingChainLen = 0;

    for (Train& t : trains) {
        if (t.isFreeSlot)
            continue;

        t.x_needToMove = false;
        for (int i = 0;; ++i) {
            if (i < g_trainDrawingChainLen) {
                if (!trainOverlaps(t, i)) {
                    // the train doesn't intersect with previously processed trains
                    // => they can be drawn in any order
                    continue;
                }
                addToDrawingChain(t, i);
                for (int j = i + 1; j < g_trainDrawingChainLen; ++j) {
                    if (trainOverlaps(t, j))
                        mergeDrawingChains(i, j);
                }
            } else {
                g_trainDrawingChains[g_trainDrawingChainLen] = nullptr;
                addToDrawingChain(t, g_trainDrawingChainLen++);
            }
            break;
        }
    }
}

/* 18fa:08d6 */
static void drawTrainList(Carriage* c)
{
    Rectangle boundingBox;

    for (; c; c = c->next) {
        const PathStep& p = g_movementPaths[c->location.chunk->type].data[c->location.pathStep];
        if ((p.angle ^ c->x_direction) == 4)
            c->direction = c->direction ^ 1;

        c->x_direction = p.angle;
        const TrainGlyph& glyph = g_trainGlyphs[c->type][p.angle][c->direction];

        boundingBox = c->rect;

        c->rect.x1 = c->location.chunk->x + p.dx - glyph.width / 2;
        c->rect.x2 = c->rect.x1 + glyph.width;
        c->drawingPriority = c->location.chunk->y + p.dy;
        c->rect.y1 = c->drawingPriority - glyph.height + g_carriageYBiases[c->type][p.angle];
        c->rect.y2 = c->rect.y1 + glyph.height;

        if (c->rect.x1 < boundingBox.x1)
            boundingBox.x1 = c->rect.x1;
        if (c->rect.x2 > boundingBox.x2)
            boundingBox.x2 = c->rect.x2;
        if (c->rect.y1 < boundingBox.y1)
            boundingBox.y1 = c->rect.y1;
        if (c->rect.y2 > boundingBox.y2)
            boundingBox.y2 = c->rect.y2;

        // TODO
        // graphics_setWriteMode1();
        // glyph = copyFromVideoMemory(glyph, c->rect.x1, c.rect.y1 + 350,
        //                             (c->rect.x2 - 1) / 8 - c->rect.x1 / 8 + 1,
        //                             glyph.height);
        // graphics_setWriteMode2();

        if (c->location.chunk->type != 6) {
            // TODO use drawSprite 1b06:067e instead
            drawGlyph(glyph.glyph1, c->rect.x1, c->rect.y1 + 350, Color::Black);
            drawGlyph(
                glyph.glyph2, c->rect.x1, c->rect.y1 + 350, entrances[c->dstEntranceIdx].bgColor
            );
            drawGlyph(
                glyph.glyph3, c->rect.x1, c->rect.y1 + 350, entrances[c->dstEntranceIdx].fgColor
            );
        }
    }
}

/* 18fa:0b73 */
static void drawTrains()
{
    for (std::uint16_t i = 0; i < g_trainDrawingChainLen; ++i)
        drawTrainList(g_trainDrawingChains[i]);
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

    drawHeaderBackground(0x15e);
    // TODO
    //     drawHeaderData(headers.trains.value,headers.money.value,headers.year.value,headers.level.value,
    //                    0x15e);
    //     drawAllDispatchers(0x15e);
    //     drawFooterWithCopyright(0x15e);
}

} // namespace resl
