#include "train.h"

#include "entrance.h"
#include "header.h"
#include "mouse/mouse_mode.h"
#include "mouse/mouse_state.h"
#include "move_trains.h"
#include "resources/carriage_bias.h"
#include "resources/movement_paths.h"
#include "resources/train_glyph.h"
#include "types/chunk.h"
#include "types/header_field.h"
#include "types/rectangle.h"
#include <graphics/color.h>
#include <graphics/drawing.h>
#include <graphics/glyph.h>
#include <graphics/vga.h>
#include <system/time.h>
#include <utility/sar.h>

#include <cassert>
#include <utility>

namespace resl {

/* 262d:5f02 : 2640 bytes */
Train g_trains[20];

/* 262d:6fd2 : 2 bytes */
std::uint16_t g_collidedTrainsArrayLen;

/* 262d:7000 : 80 bytes */
std::pair<Carriage*, Carriage*> g_collidedTrainsArray[20];

/* 262d:6fd4 : 2 bytes */
std::uint16_t g_trainDrawingChainLen;

/* 262d:6fd6 : 40 bytes */
Carriage* g_trainDrawingChains[20];

/* 262d:6ffe : 1 byte */
bool g_needToRedrawTrains;

/* 262d:7050 : 800 bytes */
Rectangle g_carriagesBoundingBoxes[100];

//-----------------------------------------------------------------------------

/* 146b:0153 */
void initTrains()
{
    for (int trainIdx = 0; trainIdx < 20; ++trainIdx) {
        for (int carriageIdx = 0; carriageIdx < 5; ++carriageIdx)
            g_trains[trainIdx].carriages[carriageIdx].train = &g_trains[trainIdx];
    }
}

/* 146b:018c */
void resetCarriages()
{
    for (int i = 0; i < 20; ++i)
        g_trains[i].isFreeSlot = true;
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

    for (Train& t : g_trains) {
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

/* 19de:0841 */
void scheduleAllTrainsRedrawing()
{
    for (std::int16_t i = 0; i < sizeof(g_trains) / sizeof(*g_trains); ++i)
        g_trains[i].needToRedraw = true;
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
void drawTrains()
{
    for (std::uint16_t i = 0; i < g_trainDrawingChainLen; ++i)
        drawTrainList(g_trainDrawingChains[i]);
}

/* 132d:0002 */
void eraseTrain(const Train& train)
{
    assert(mouse::g_state.mode);
    mouse::g_state.mode->clearFn();

    drawing::setVideoModeR0W1();
    for (uint8_t i = 0; i < train.carriageCnt; ++i)
        drawing::copyFromShadowBuffer(train.carriages[i].rect);
    drawing::setVideoModeR0W2();

    mouse::g_state.mode->drawFn();
}

/* 19de:0797 */
static Train* allocateTrainSlot()
{
    for (Train& t : g_trains) {
        if (t.isFreeSlot) {
            t.isFreeSlot = false;
            return &t;
        }
    }
    return nullptr;
}

/* 1a65:0256 */
Train* spawnServer(std::int16_t entranceIdx)
{
    Train* train = allocateTrainSlot();
    if (train) {
        Entrance& entrance = g_entrances[entranceIdx];

        train->carriageCnt = 1;
        train->x_maxSpeed = 5;     // TODO use constants from the array 1d32:0000
        train->x_acceleration = 5; // TODO use constants from the array 1d32:0000
        train->carriages[0].type = CarriageType::Server;

        train->head.forwardDirection = true;
        train->head.chunk = &entrance.chunk;
        train->head.pathStep = 0;

        train->carriages[0].dstEntranceIdx = serverEntranceIdx;
        train->carriages[0].location = train->head;

        moveAlongPath(train->carriages[0].location,
                      g_trainGlyphs[0][0][0].width / 2 + 4);

        // In the original game they use a global variable here for some reason,
        // but they don't use it anywhere else
        Location loc = train->carriages[0].location;
        moveAlongPath(loc, g_trainGlyphs[0][0][0].width / 2 + 4);
        train->tail = loc;

        setEmptyRectangle(train->carriages[0].rect);

        train->head.forwardDirection = false;
        train->tail.forwardDirection = false;
        train->carriages[0].location.forwardDirection = false;

        moveAlongPath(train->head, 60);
        moveAlongPath(train->tail, 60);
        moveAlongPath(train->carriages[0].location, 60);

        train->year = g_headers[static_cast<int>(HeaderFieldId::Year)].value;
        train->lastMovementTime = getTime();
        train->needToRedraw = true;

        train->carriages[0].direction = entrance.chunk.x > 320;
        train->carriages[0].x_direction = 0;
        train->x_headCarriageIdx = 0;
    }
    return train;
}

} // namespace resl
