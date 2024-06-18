#include "train.h"

#include "chunk.h"
#include "draw_header.h"
#include "entrance.h"
#include "header.h"
#include "mouse/mouse_mode.h"
#include "mouse/mouse_state.h"
#include "move_trains.h"
#include "resources/carriage_bias.h"
#include "resources/movement_paths.h"
#include "resources/train_glyph.h"
#include "resources/train_specification.h"
#include "status_bar.h"
#include "types/header_field.h"
#include "types/rectangle.h"
#include <graphics/color.h>
#include <graphics/drawing.h>
#include <graphics/glyph.h>
#include <graphics/vga.h>
#include <system/random.h>
#include <system/time.h>
#include <tasks/task.h>
#include <utility/sar.h>

#include <array>
#include <cassert>
#include <cstdlib>
#include <utility>

namespace resl {

/* 262d:5f02 : 2640 bytes */
std::array<Train, 20> g_trains;

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
    for (Train& train : g_trains) {
        for (int carriageIdx = 0; carriageIdx < 5; ++carriageIdx)
            train.carriages[carriageIdx].train = &train;
    }
}

/* 146b:018c */
void resetCarriages()
{
    for (Train& train : g_trains)
        train.isFreeSlot = true;
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
    for (Train& train : g_trains)
        train.needToRedraw = true;
}

/* 18fa:08d6 */
void drawTrainList(Carriage* c)
{
    Rectangle* curBoundingBox = g_carriagesBoundingBoxes;
    vga::VideoMemPtr shadowBufPtr = drawing::VIDEO_MEM_SHADOW_BUFFER;

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

        vga::setVideoModeR0W1();
        shadowBufPtr = drawing::copySpriteToShadowBuffer(shadowBufPtr, c->rect.x1, c->rect.y1 + 350,
                                                         sar<std::int16_t>(c->rect.x2 - 1, 3) - sar(c->rect.x1, 3) + 1,
                                                         glyph.height);
        vga::setVideoModeR0W2();

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

    vga::setVideoModeR0W1();
    for (uint8_t i = 0; i < train.carriageCnt; ++i)
        drawing::copyFromShadowBuffer(train.carriages[i].rect);
    vga::setVideoModeR0W2();

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

static std::int16_t getTrainSpawnInterval()
{
    std::int16_t res = 18000;
    for (std::int16_t i = 0; i < g_headers[static_cast<int>(HeaderFieldId::Level)].value - 1; ++i)
        res -= res / 25;
    return res;
}

/* 19de:07b7 */
static bool noTrainsExist()
{
    for (const Train& train : g_trains) {
        if (!train.isFreeSlot)
            return false;
    }
    return true;
}

/* 1a65:03fa */
static bool isTrainAvailableInCurrentYear(CarriageType type)
{
    std::int16_t year = g_headers[static_cast<int>(HeaderFieldId::Year)].value;
    const TrainSpecification& spec = g_trainSpecifications[type];
    return year >= spec.minYear && year <= spec.maxYear;
}

/* 1a65:043a */
static void generateTrain(Train& t, std::int16_t entranceIdx)
{
    t.carriageCnt = genRandomNumber(5) + 1;
    do {
        std::int16_t rnd = genRandomNumber(6);
        t.carriages[0].type =
            static_cast<CarriageType>(rnd + CarriageType::AncientLocomotive);
    } while (!isTrainAvailableInCurrentYear(t.carriages[0].type));

    t.maxSpeed = g_trainSpecifications[t.carriages[0].type].maxSpeed;
    t.speed = t.maxSpeed;
    t.carriages[0].direction = g_entrances[entranceIdx].chunk.x > 320;
    t.carriages[0].x_direction = 0;
    for (std::uint8_t i = 1; i < t.carriageCnt; ++i) {
        do {
            t.carriages[i].type =
                g_trainSpecifications[t.carriages[0].type].possibleCarriages[genRandomNumber(5)];
        } while (!isTrainAvailableInCurrentYear(t.carriages[i].type));
    }
}

/* 1a65:0100 */
static Train* spawnTrain(std::int16_t entranceIdx)
{
    Train* t = allocateTrainSlot();
    if (t) {
        generateTrain(*t, entranceIdx);
        t->head.chunk = &g_entrances[entranceIdx].chunk;
        t->head.forwardDirection = false;
        t->head.pathStep = 0;
        moveAlongPath(t->head, 60);

        std::int16_t dstEntranceIdx;
        do {
            dstEntranceIdx = genRandomNumber(g_entranceCount);
        } while (dstEntranceIdx == entranceIdx);

        // In the original game they use a global variable here for some reason,
        // but they don't use it anywhere else
        Location loc = t->head;
        loc.forwardDirection = !loc.forwardDirection;
        for (std::uint8_t i = 0; i < t->carriageCnt; ++i) {
            Carriage& c = t->carriages[i];
            c.dstEntranceIdx = dstEntranceIdx;

            const std::int16_t moveStep = g_trainGlyphs[c.type][0][0].width / 2 + 4;
            moveAlongPath(loc, moveStep);
            c.location = loc;
            c.location.forwardDirection = !c.location.forwardDirection;
            setEmptyRectangle(c.rect);
            moveAlongPath(loc, moveStep);
        }
        t->tail = loc;
        t->tail.forwardDirection = !t->tail.forwardDirection;

        t->needToRedraw = true;
        t->headCarriageIdx = 0;
        t->year = g_headers[static_cast<int>(HeaderFieldId::Year)].value;
        t->lastMovementTime = getTime();
    }
    return t;
}

/* 16a6:08bb */
void tryRunWaitingTrains()
{
    for (std::int16_t i = g_entranceCount - 1; i >= 0; --i) {
        if (g_entrances[i].waitingTrainsCount && entranceIsFree(i)) {
            Train* train = spawnTrain(i);
            if (train && !(--g_entrances[i].waitingTrainsCount))
                drawDispatcher(i, false);
        }
    }
}

/* 1a65:0073 */
static void spawnNewTrain()
{
    const std::int16_t entranceIdx = genRandomNumber(g_entranceCount);
    if (g_entranceCount < 2)
        return;

    if (entranceIsFree(entranceIdx)) {
        if (Train* train = spawnTrain(entranceIdx)) {
            const std::uint16_t level =
                static_cast<std::uint16_t>(g_headers[static_cast<int>(HeaderFieldId::Level)].value);
            if (level >= 6 && level * 200 > static_cast<std::uint16_t>(std::rand())) {
                // create an unmanaged blinking train
                for (std::uint8_t i = 0; i < train->carriageCnt; ++i)
                    train->carriages[i].dstEntranceIdx = blinkingTrainEntranceIdx;
            }
            return;
        }
        showStatusMessage("Railnet OVERFLOWED. New train waits outside");
    }
    addWaitingTrain(entranceIdx);
}

/* 1a65:0030 */
Task taskSpawnTrains()
{
    for (;;) {
        const std::uint16_t period = getTrainSpawnInterval();

        if (!noTrainsExist())
            co_await sleep(genRandomNumber(period / 2) + 1);

        spawnNewTrain();
        co_await sleep(genRandomNumber(period / 2) + 1);
    }
}

/* 1a65:0256 */
Train* spawnServer(std::int16_t entranceIdx)
{
    Train* train = allocateTrainSlot();
    if (train) {
        Entrance& entrance = g_entrances[entranceIdx];

        train->carriageCnt = 1;
        train->maxSpeed = g_trainSpecifications[0].maxSpeed;
        train->speed = train->maxSpeed;
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
        train->headCarriageIdx = 0;
    }
    return train;
}

/* 16a6:06e0 */
void accelerateTrains(std::int16_t count)
{
    /* 262d:6f4e : 1 byte */
    static std::uint8_t g_lastProcessedTrain = 0;

    /* 262d:6f3a : 20 bytes */
    static std::uint8_t g_lastProcessedCarriages[g_trains.size()] = {};

    for (int16_t i = 0; i < count; ++i) {
        g_lastProcessedTrain = (g_lastProcessedTrain + 1) % g_trains.size();

        Train& train = g_trains[g_lastProcessedTrain];
        if (!train.isFreeSlot) {
            std::uint8_t c = ++g_lastProcessedCarriages[g_lastProcessedTrain];
            if (c % train.carriageCnt == 0 && train.speed < train.maxSpeed)
                ++train.speed;
        }
    }
}

/* 16a6:0893 */
std::int16_t waitingTrainsCount()
{
    std::int16_t res = 0;
    for (std::int16_t i = 0; i < g_entranceCount; ++i)
        res += g_entrances[i].waitingTrainsCount;
    return res;
}

} // namespace resl
