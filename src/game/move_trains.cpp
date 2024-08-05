#include "move_trains.h"

#include "drawing.h"
#include "entrance.h"
#include "header.h"
#include "header_field.h"
#include "melody.h"
#include "mouse/management_mode.h"
#include "mouse/mode.h"
#include "mouse/state.h"
#include "rail.h"
#include "resources/movement_paths.h"
#include "resources/train_glyph.h"
#include "semaphore.h"
#include "train.h"
#include <graphics/color.h>
#include <graphics/drawing.h>
#include <graphics/vga.h>
#include <system/active_sleep.h>
#include <system/random.h>
#include <system/sound.h>
#include <system/time.h>
#include <tasks/task.h>
#include <types/position.h>
#include <types/rectangle.h>
#include <ui/components/status_bar.h>
#include <utility/sar.h>

#include <cstdlib>
#include <utility>

namespace resl {

/* 1d7d:2122 */
Task g_taskMoveAndRedrawTrains;

//-----------------------------------------------------------------------------

enum class MoveAbility {
    Ok,
    // the train has reached a impasse or blocked switch
    BlockedByRoad,
    // the train has reached a red semaphore
    BlockedBySemaphore
};

/* 18a5:0006 */
static MoveAbility tryMoveAlongPath(Location& loc)
{
    if (loc.forwardDirection) {
        if (loc.pathStep < loc.rail->maxPathStep) {
            ++loc.pathStep;
            return MoveAbility::Ok;
        }
    } else {
        if (loc.pathStep > loc.rail->minPathStep) {
            --loc.pathStep;
            return MoveAbility::Ok;
        }
    }

    // in the original code:
    // "loc.rail->connections[loc.forwardDirection].rail & (~g_disabledSwitchPath)"
    if (!loc.rail->connections[loc.forwardDirection].rail ||                      // impasse
        loc.rail->connections[loc.forwardDirection].rail == g_disabledSwitchPath) // blinking train
        return MoveAbility::BlockedByRoad;

    std::int8_t semSlotId = loc.rail->semSlotIdByDirection[loc.forwardDirection];
    if (semSlotId != -1 && g_semaphores[semSlotId].isRed)
        return MoveAbility::BlockedBySemaphore;

    const RailConnection& next = loc.rail->connections[loc.forwardDirection];
    if (next.slot)
        loc.pathStep = g_movementPaths[next.rail->type].size - 1;
    else
        loc.pathStep = 0;
    loc.rail = next.rail;
    loc.forwardDirection = next.slot ^ 1;

    return MoveAbility::Ok;
}

/* 18a5:00c1 */
void moveAlongPath(Location& loc, std::int16_t distance)
{
    for (std::int16_t i = 0; i < distance; ++i) {
        if (loc.forwardDirection) {
            if (loc.pathStep < loc.rail->maxPathStep) {
                ++loc.pathStep;
                continue;
            }
        } else {
            if (loc.pathStep > loc.rail->minPathStep) {
                --loc.pathStep;
                continue;
            }
        }

        const RailConnection& next = loc.rail->connections[loc.forwardDirection];
        if (next.slot)
            loc.pathStep = g_movementPaths[next.rail->type].size - 1;
        else
            loc.pathStep = 0;
        loc.rail = next.rail;
        loc.forwardDirection = next.slot ^ 1;
    }
}

/* 18a5:044c */
static std::int16_t countPassengerCarriages(const Train& train)
{
    std::int16_t res = 0;
    for (std::uint8_t i = 0; i < train.carriageCnt; ++i) {
        switch (train.carriages[i].type) {
        case CarriageType::HighSpeedLocomotive:
        case CarriageType::AncientPassengerCarriage:
        case CarriageType::PassengerCarriage:
        case CarriageType::HighSpeedPassengerCarriage:
            ++res;
            break;
        default:
            break;
        }
    }
    return res;
}

/* 18a5:050c */
static Position carriagePosition(const Location& loc)
{
    const PathStep& p = g_movementPaths[loc.rail->type].data[loc.pathStep];
    return { static_cast<std::int16_t>(loc.rail->x + p.dx),
             static_cast<std::int16_t>(loc.rail->y + p.dy) };
}

/* 132d:0239 */
static void animateCollisionAndPlaySound(Position pos)
{
    vga::setDataRotation(0x18); // rotation = 0, mode = XOR

    std::int16_t x1[125];
    std::int16_t y1[125];
    std::int16_t x2[125];
    std::int16_t y2[125];

    for (int i = 0; i < 125; ++i) {
        x1[i] = pos.x + symmetricRand(5);
        y1[i] = pos.y + symmetricRand(3);
        x2[i] = pos.x + symmetricRand(i / 2 + 1);
        y2[i] = pos.y + symmetricRand(i / 2 + 1);

        graphics::line(x1[i], y1[i], x2[i], y2[i], Color::White);

        std::int16_t freq = genRandomNumber(i * 16 + 1);
        sound(freq + 30);
        activeSleep(1);
    }
    for (int i = 0; i < 125; ++i) {
        graphics::line(x1[i], y1[i], x2[i], y2[i], Color::White);

        std::int16_t freq = genRandomNumber(i * 16 + 1);
        sound(freq + 30);
        activeSleep(1);
    }
    nosound();

    vga::setDataRotation(0); // default mode - simple copying without rotation
}

/* 18fa:023d */
static void createCrashMarker(Carriage& c)
{
    Train& train = *c.train;
    train.isFreeSlot = false;
    train.headCarriageIdx = 0;
    train.carriageCnt = 1;
    train.carriages[0] = c;
    train.carriages[0].dstEntranceIdx = crashedTrainEntranceIdx;
    train.carriages[0].type = CarriageType::CrashedTrain;
    train.tail = train.carriages[0].location;
    train.head = train.carriages[0].location;
    setEmptyRectangle(train.carriages[0].rect);
    train.needToRedraw = true;
}

/* 18fa:063d */
static void showPassengerAccidentMessage(std::int16_t count)
{
    char msg[] = "?? PASSENGER CARS involved in the accident";
    std::int16_t firstDigit = count / 10;
    msg[0] = firstDigit ? '0' + firstDigit : ' ';
    msg[1] = '0' + count % 10;
    showStatusMessage(msg);
}

/* 18fa:00ba */
static void removeTrainFromDrawingChain(Train& train)
{
    Carriage** c = &g_trainDrawingChains[train.drawingChainIdx];
    while (*c) {
        if ((*c)->train == &train)
            *c = (*c)->next;
        else
            c = &((*c)->next);
    }
}

/* 18a5:042f */
static void deleteTrain(Train& train)
{
    removeTrainFromDrawingChain(train);
    eraseTrain(train);
    train.isFreeSlot = true;
}

/* 18a5:03a5 */
static void reverseTrain(Train& train)
{
    std::swap(train.tail, train.head);
    for (std::uint8_t i = 0; i < train.carriageCnt; ++i)
        train.carriages[i].location.forwardDirection = !train.carriages[i].location.forwardDirection;
    train.head.forwardDirection = !train.head.forwardDirection;
    train.tail.forwardDirection = !train.tail.forwardDirection;
    if (train.headCarriageIdx == 0)
        train.headCarriageIdx = train.carriageCnt - 1;
    else
        train.headCarriageIdx = 0;
    train.speed = 1;
}

/* 18a5:0145 */
bool moveTrain(Train& train, std::int16_t dTime)
{
    const std::int16_t totalDistance = train.speed * dTime * 10 + train.movementDebt;
    const std::int16_t maxDistance = totalDistance >> 8;
    train.movementDebt = static_cast<std::uint8_t>(totalDistance);

    std::int16_t distance = 0;
    for (; distance < maxDistance; ++distance) {
        MoveAbility moveAbility = tryMoveAlongPath(train.head);
        if (moveAbility == MoveAbility::Ok)
            g_needToRedrawTrains = true;
        else {
            // blinking trains ignore semaphores
            if (moveAbility != MoveAbility::BlockedBySemaphore || train.carriages[0].dstEntranceIdx != blinkingTrainEntranceIdx)
                break;
            moveAlongPath(train.head, 1);
        }
    }

    for (std::uint8_t i = 0; i < train.carriageCnt; ++i)
        moveAlongPath(train.carriages[i].location, distance);
    moveAlongPath(train.tail, distance);

    if (distance < maxDistance) {
        // the train faced some obstacle
        if (train.carriages[0].dstEntranceIdx == blinkingTrainEntranceIdx) {
            // the blinking train crashes on blocked switches
            const std::int16_t passengerCnt = countPassengerCarriages(train);

            animateCollisionAndPlaySound(carriagePosition(train.head));
            eraseTrain(train);
            train.carriages[0].location = train.head;
            createCrashMarker(train.carriages[train.headCarriageIdx]);

            if (passengerCnt)
                showPassengerAccidentMessage(passengerCnt);

            spendMoney(train.carriageCnt + passengerCnt * 9);
            return true;
        }
    } else {
        const Entrance& dstEntrance = g_entrances[train.carriages[0].dstEntranceIdx];
        if (train.head.rail == dstEntrance.rail.connections[0].rail)
            train.maxSpeed = 30;
        if (train.head.rail->type != g_innerEntranceRailType)
            return false;
        const Rectangle& rect = train.carriages[train.carriageCnt - 1 - train.headCarriageIdx].rect;
        if (rect.x1 < 638 && rect.x2 > 1)
            return false;

        if (train.head.rail == &dstEntrance.rail ||
            train.carriages[0].type == CarriageType::Server ||
            train.carriages[0].dstEntranceIdx == blinkingTrainEntranceIdx) {
            // the train has reached the entrance
            deleteTrain(train);
            drawEraseTrainFinishedExclamation((rect.x1 + rect.x2) / 2, (rect.y1 + rect.y2) / 2);
            playTrainFinishedMelody(train.carriageCnt);
            if (train.carriages[0].type == CarriageType::Server)
                showStatusMessage("Server went out");
            else {
                startHeaderFieldAnimation(HeaderFieldId::Trains, 1);
                startHeaderFieldAnimation(HeaderFieldId::Money, train.carriageCnt + 3);
            }
            // this function uses the XOR function when drawing, thus this duplicate call simply
            // erases the previously drawn image
            drawEraseTrainFinishedExclamation((rect.x1 + rect.x2) / 2, (rect.y1 + rect.y2) / 2);
            return false;
        }
    }
    reverseTrain(train);
    return false;
}

/* 18fa:0295 */
void processCarriageCollision(Carriage* c1, Carriage* c2, std::int16_t x, std::int16_t y)
{
    for (;;) {
        if (c1->type == CarriageType::Server) {
            if (c2->type == CarriageType::CrashedTrain) {
                // server should fix the damaged road
                if (c1->train->speed) {
                    c1->train->speed = 0;
                    return;
                }
                playFixRoadMelody();
                deleteTrain(*c2->train);
                c1->train->speed = 1;
                spendMoney(1);
                return;
            }
        }
        if (c1->type == CarriageType::CrashedTrain) {
            if (c2->type == CarriageType::CrashedTrain)
                return;
            std::swap(c1, c2);
            continue;
        }
        break;
    }

    animateCollisionAndPlaySound({ x, y });
    std::int16_t nPassenger = countPassengerCarriages(*c1->train);
    std::int16_t dMoney = c1->train->carriageCnt;

    deleteTrain(*c1->train);
    if (c2->type == CarriageType::CrashedTrain)
        c2->train->needToRedraw = true;
    else {
        deleteTrain(*c2->train);
        if (x > 0 && x < 639)
            createCrashMarker(*c1);
        dMoney += c2->train->carriageCnt;
        nPassenger += countPassengerCarriages(*c2->train);
    }

    if (nPassenger)
        showPassengerAccidentMessage(nPassenger);

    dMoney += nPassenger * 9;
    spendMoney(dMoney);
}

/*g */
static bool handleCollisions(Carriage& c1, Carriage& c2)
{
    /* 2D rotation:

        x' = x cos(a) - y sin(a)
        y' = x sin(a) + y cos(a)
     */

    /* 1d7d:234c - 10 bytes */
    static const std::int16_t g_sinArr[5] = { 0, 3, 5, 5, 3 };

    /* 1d7d:2356 - 10 bytes */
    static const std::int16_t g_cosArr[5] = { 5, 4, 1, -1, -4 };

    const Location& loc1 = c1.location;
    const Location& loc2 = c2.location;

    const PathStep& p1 = g_movementPaths[loc1.rail->type].data[loc1.pathStep];
    const PathStep& p2 = g_movementPaths[loc2.rail->type].data[loc2.pathStep];

    const std::int16_t x1 = loc1.rail->x + p1.dx;
    const std::int16_t y1 = loc1.rail->y + p1.dy;

    const std::int16_t x2 = loc2.rail->x + p2.dx;
    const std::int16_t y2 = loc2.rail->y + p2.dy;

    const std::int16_t dx = x1 - x2;
    const std::int16_t dy = (y1 - y2) * 4;

    const std::int16_t cos = g_cosArr[p1.angle];
    const std::int16_t sin = -g_sinArr[p1.angle];

    const std::int16_t w1 =
        g_trainGlyphs[static_cast<int>(c1.type)][0][0].width - 4;
    const std::int16_t w2 =
        g_trainGlyphs[static_cast<int>(c2.type)][0][0].width - 4;

    const std::int16_t a1 = (cos * w1) / 10;
    const std::int16_t b1 = (sin * w1) / 10;

    const std::int16_t a2 = (sin * 8) / 10;
    const std::int16_t b2 = (-cos * 8) / 10;

    std::int16_t xOffsets[5];
    std::int16_t yOffsets[5];

    xOffsets[0] = a1 - a2;
    yOffsets[0] = b1 - b2;
    xOffsets[1] = a1 + a2;
    yOffsets[1] = b1 + b2;
    xOffsets[2] = -a1 - a2;
    yOffsets[2] = -b1 - b2;
    xOffsets[3] = -a1 + a2;
    yOffsets[3] = -b1 + b2;

    // This is wierd, but in the original game this values is overwritten with 0:
    //    18fa:0563
    xOffsets[0] = 0;

    // The original game makes 5 iterations in the loop (18fa:062d).
    // But I don't see any place where they initialize xOffsets[4] and yOffsets[4].
    // So, if I'm not mistaken, they have reading of uninitialized memory here 🤷🏻
    for (int i = 0; i < 4; ++i) {
        std::int16_t curX = xOffsets[i] + dx;
        std::int16_t curY = yOffsets[i] + dy;

        const std::int16_t xDist = g_sinArr[c2.x_direction] * curX + g_cosArr[c2.x_direction] * curY;
        const std::int16_t yDist = g_cosArr[c2.x_direction] * curX - g_sinArr[c2.x_direction] * curY;
        if (std::abs(xDist) > 20 || std::abs(yDist) >= (w2 * 5) / 2)
            continue;

        processCarriageCollision(&c1, &c2, (x1 + x2) / 2, (y1 + y2) / 2);
        return true;
    }
    return false;
}

/* 18fa:0b10 */
static void eraseCarriagesInShadowBuffer(const Carriage& c, vga::VideoMemPtr ptr)
{
    std::int16_t widthBytes = sar<std::int16_t>((c.rect.x2 - 1), 3) - sar(c.rect.x1, 3) + 1;
    std::int16_t height = c.rect.y2 - c.rect.y1;
    if (c.next)
        eraseCarriagesInShadowBuffer(*c.next, ptr + widthBytes * height);
    graphics::copySpriteFromShadowBuffer(ptr, c.rect.x1, c.rect.y1 + 350, widthBytes, height);
}

/* 18fa:068a */
Task taskMoveAndRedrawTrains()
{
    constexpr TimeT minMovementPeriod = 15;

    // Hmm... interesting, it looks like the original game starts with an
    // uninitialized variable here.
    // The first access to the variable is INC operation:
    //       18fa:0719 ff 46      INC     word ptr [BP + -0x2]
    // But in the loop we have a logic that bounds the value
    // => it should still work regardless the initial value.
    std::uint16_t curChain = 0;

    for (;;) {
        co_await sleep(1);

        scheduleTrainsDrawing();

        for (std::uint16_t i = 0; i < g_collidedTrainsArrayLen; ++i) {
            const auto [c1, c2] = g_collidedTrainsArray[i];
            if (c1->train->isFreeSlot || c2->train->isFreeSlot)
                continue;
            if (!handleCollisions(*c1, *c2))
                handleCollisions(*c2, *c1);
        }

        for (std::uint16_t i = 0; i < g_trainDrawingChainLen; ++i) {
            co_await yield();

            bool needToRedrawCursor = false;
            g_needToRedrawTrains = false;

            if (++curChain >= g_trainDrawingChainLen)
                curChain = 0;

            Carriage* carriage = g_trainDrawingChains[curChain];
            if (!carriage || (getTime() - carriage->train->lastMovementTime < minMovementPeriod))
                continue;

            bool needToRestartDrawing = false;
            do {
                if (carriage->train->needToRedraw) {
                    g_needToRedrawTrains = true;
                    carriage->train->needToRedraw = false;
                }
                if (!carriage->train->isActualPosition) {
                    carriage->train->isActualPosition = true;
                    const TimeT curTime = getTime();
                    const TimeT dTime = curTime - carriage->train->lastMovementTime;
                    carriage->train->lastMovementTime = curTime;

                    if (carriage->type != CarriageType::CrashedTrain) {
                        bool trainCrashed = moveTrain(*carriage->train, dTime);
                        if (trainCrashed) {
                            needToRestartDrawing = true;
                            break;
                        }
                    }
                }

                const std::int16_t mouseX = mouse::g_state.mode->x & 0xFFF8;
                const std::int16_t mouseY = mouse::g_state.mode->y;
                if (mouse::g_state.mode == &mouse::g_modeManagement &&
                    mouseX - 8 < carriage->rect.x2 && mouseX + 32 > carriage->rect.x1 &&
                    mouseY < carriage->rect.y2 && mouseY + 16 > carriage->rect.y1)
                    needToRedrawCursor = true;

                carriage = carriage->next;
            } while (carriage);
            if (needToRestartDrawing)
                break;

            carriage = g_trainDrawingChains[curChain];
            if (!g_needToRedrawTrains || !carriage)
                continue;

            const Rectangle* curCarriageRect = g_carriagesBoundingBoxes;

            drawTrainList(carriage);
            for (std::uint16_t i = 0; i < g_semaphoreCount; ++i) {
                if (g_semaphores[i].isRightDirection)
                    drawSemaphore(g_semaphores[i], 350);
            }

            vga::waitVerticalRetrace();
            if (needToRedrawCursor)
                mouse::g_state.mode->clearFn();

            vga::setVideoModeR0W1();
            do {
                graphics::copyFromShadowBuffer(*curCarriageRect++);
                carriage = carriage->next;
            } while (carriage);
            vga::setVideoModeR0W2();

            if (needToRedrawCursor)
                mouse::g_state.mode->drawFn();

            vga::setVideoModeR0W1();
            eraseCarriagesInShadowBuffer(*g_trainDrawingChains[curChain], graphics::VIDEO_MEM_SHADOW_BUFFER);
            vga::setVideoModeR0W2();
        }
    }
    co_return;
}

} // namespace resl
