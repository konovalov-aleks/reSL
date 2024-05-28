#include "drawing.h"

#include "draw_header.h"
#include "draw_impasse.h"
#include "game_data.h"
#include "graphics/drawing.h"
#include "header.h"
#include "resources/carriage_bias.h"
#include "resources/movement_paths.h"
#include "resources/rail_glyph.h"
#include "resources/semaphore_glyph.h"
#include "resources/static_object_glyph.h"
#include "resources/train_finished_exclamation_glyph.h"
#include "resources/train_glyph.h"
#include "types/chunk.h"
#include "types/entrance.h"
#include "types/header_field.h"
#include "types/rail_info.h"
#include "types/rectangle.h"
#include "types/semaphore.h"
#include "types/static_object.h"
#include "types/switch.h"
#include "types/train.h"
#include <graphics/color.h>
#include <graphics/glyph.h>
#include <graphics/text.h>
#include <graphics/vga.h>
#include <system/buffer.h>
#include <system/driver/driver.h>
#include <system/random.h>
#include <system/read_file.h>
#include <utility/sar.h>

#include <algorithm>
#include <cassert>
#include <cstdint>
#include <cstdlib>
#include <iterator>
#include <utility>

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

/* 19de:0841 */
void scheduleAllTrainsRedrawing()
{
    for (std::int16_t i = 0; i < sizeof(trains) / sizeof(*trains); ++i)
        trains[i].needToRedraw = true;
}

/* 13d1:010f */
void drawSwitch(std::int16_t idx, bool drawToScreen)
{
    auto& vga = Driver::instance().vga();

    VideoMemPtr dstPtr = VIDEO_MEM_START_ADDR + (idx + 1) * 30 - 1;
    // TODO implement properly!!!
    Switch& s = g_switches[idx];
    Chunk* rail = s.curChunk.chunk;
    const RailGlyph* rg = railBackgrounds[rail->type].switches[s.curChunk.slot];

    drawing::setVideoModeR0W1();
    const std::uint8_t* glyphData = rg->glyph.data;
    for (std::uint8_t y = 0; y < rg->glyph.height; ++y) {
        std::int16_t yPos = rail->y + rg->dy + y;
        for (std::uint8_t xBytes = 0; xBytes < rg->glyph.width; ++xBytes) {
            if (*glyphData) {
                std::int16_t xPos = rail->x + rg->dx + xBytes * 8;
                VideoMemPtr srcPtr = VIDEO_MEM_START_ADDR + (yPos + 350) * VIDEO_MEM_ROW_BYTES + sar(xPos, 3);
                vga.write(dstPtr++, vga.read(srcPtr));
            }
            ++glyphData;
        }
    }
    drawing::setVideoModeR0W2();

    if (drawToScreen)
        drawGlyphAlignX8(&rg->glyph, rail->x + rg->dx, rail->y + rg->dy, Color::Black);

    drawGlyphAlignX8(&rg->glyph, rail->x + rg->dx, rail->y + rg->dy + 350, Color::Black);
}

/* 13d1:026c */
void drawSwitch2(std::int16_t idx, std::int16_t yOffset)
{
    const Switch& s = g_switches[idx];
    const Chunk* rail = s.curChunk.chunk;
    const RailGlyph* rg = railBackgrounds[rail->type].switches[s.curChunk.slot];
    drawGlyphAlignX8(&rg->glyph, rail->x + rg->dx, rail->y + rg->dy + yOffset, Color::Black);
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
        s.isRed ? Color::Red : Color::LightGreen);
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

/* 18fa:0142 */
static bool trainOverlaps(Train& t, int idx)
{
    for (int i = t.carriageCnt - 1; i >= 0; --i) {
        Carriage& c1 = t.carriages[i];
        for (Carriage* c2 = g_trainDrawingChains[idx]; c2; c2 = c2->next) {
            // Check if the rectangles overlap
            if (c1.rect.y1 < c2->rect.y2 && c1.rect.y2 > c2->rect.y1 &&
                (c1.rect.x1 & ~7) <= (c2->rect.x2 & ~7) &&
                (c1.rect.x2 & ~7) >= (c2->rect.x1 & ~7)) {

                g_collidedTrainsArray[g_collidedTrainsArrayLen++] = { &c1, c2 };
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
void scheduleTrainsDrawing()
{
    g_collidedTrainsArrayLen = 0;
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
void drawTrainList(Carriage* c)
{
    Rectangle* curBoundingBox = g_carriagesBoundingBoxes;
    VideoMemPtr shadowBufPtr = drawing::VIDEO_MEM_SHADOW_BUFFER;

    for (; c; c = c->next) {
        const PathStep& p = g_movementPaths[c->location.chunk->type].data[c->location.pathStep];
        if ((p.angle ^ c->x_direction) == 4)
            c->direction = c->direction ^ 1;

        c->x_direction = p.angle;
        const TrainGlyph& glyph = g_trainGlyphs[c->type][p.angle][c->direction];

        *curBoundingBox = c->rect;

        c->rect.x1 = c->location.chunk->x + p.dx - glyph.width / 2;
        c->rect.x2 = c->rect.x1 + glyph.width;
        c->drawingPriority = c->location.chunk->y + p.dy;
        c->rect.y1 = c->drawingPriority - glyph.height + g_carriageYBiases[c->type][p.angle];
        c->rect.y2 = c->rect.y1 + glyph.height;

        if (c->rect.x1 < curBoundingBox->x1)
            curBoundingBox->x1 = c->rect.x1;
        if (c->rect.x2 > curBoundingBox->x2)
            curBoundingBox->x2 = c->rect.x2;
        if (c->rect.y1 < curBoundingBox->y1)
            curBoundingBox->y1 = c->rect.y1;
        if (c->rect.y2 > curBoundingBox->y2)
            curBoundingBox->y2 = c->rect.y2;

        drawing::setVideoModeR0W1();
        shadowBufPtr = drawing::copySpriteToShadowBuffer(shadowBufPtr, c->rect.x1, c->rect.y1 + 350,
                                                         sar<std::int16_t>(c->rect.x2 - 1, 3) - sar(c->rect.x1, 3) + 1,
                                                         glyph.height);
        drawing::setVideoModeR0W2();

        if (c->location.chunk->type != 6) {
            drawGlyph(glyph.glyph1, c->rect.x1, c->rect.y1 + 350, Color::Black);
            drawGlyph(glyph.glyph2, c->rect.x1, c->rect.y1 + 350, g_entrances[c->dstEntranceIdx].bgColor);
            drawGlyph(glyph.glyph3, c->rect.x1, c->rect.y1 + 350, g_entrances[c->dstEntranceIdx].fgColor);
        }

        ++curBoundingBox;
    }
}

/* 18fa:0b73 */
static void drawTrains()
{
    for (std::uint16_t i = 0; i < g_trainDrawingChainLen; ++i)
        drawTrainList(g_trainDrawingChains[i]);
}

/* 12ba:0097 */
static void drawCopyright(std::int16_t yOffset)
{
    drawTextSmall(12, 336 + yOffset,
                  " * SHORTLINE * Game by Andrei Snegov * (c) DOKA 1992 Moscow * Version 1.1 *",
                  Color::Black);
}

/* 132d:013c */
static void drawFooterWithCopyright(std::int16_t yOffset)
{
    drawing::filledRectangle(0, 334 + yOffset, 80, 16, 0xFF, Color::Gray);
    drawing::filledRectangle(0, 334 + yOffset, 80, 1, 0xFF, Color::Black);
    drawCopyright(yOffset);
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

    drawFooterWithCopyright(350);
}

/* 132d:0002 */
void eraseTrain(const Train& train)
{
    // TODO draw cursor?

    drawing::setVideoModeR0W1();
    for (uint8_t i = 0; i < train.carriageCnt; ++i)
        drawing::copyFromShadowBuffer(train.carriages[i].rect);
    drawing::setVideoModeR0W2();

    // TODO draw cursor?
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
