#include "move_trains.h"

#include "drawing.h"
#include "game_data.h"
#include "header.h"
#include "resources/movement_paths.h"
#include "types/entrance.h"
#include "types/header_field.h"
#include "types/position.h"
#include <graphics/drawing.h>
#include <system/time.h>
#include <utility/sar.h>

#include <utility>

namespace resl {

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
        if (loc.pathStep < loc.chunk->maxPathStep) {
            ++loc.pathStep;
            return MoveAbility::Ok;
        }
    } else {
        if (loc.pathStep > loc.chunk->minPathStep) {
            --loc.pathStep;
            return MoveAbility::Ok;
        }
    }

    // in the original code:
    // "p.chunk->x_neighbours[p.forwardDirection].chunk & (~specialChunkPtr)"
    if (!loc.chunk->x_neighbours[loc.forwardDirection].chunk ||                 // impasse
        loc.chunk->x_neighbours[loc.forwardDirection].chunk == specialChunkPtr) // blinking train
        return MoveAbility::BlockedByRoad;

    std::int8_t semSlotId = loc.chunk->semSlotIdByDirection[loc.forwardDirection];
    if (semSlotId != -1 && g_semaphores[semSlotId].isRed)
        return MoveAbility::BlockedBySemaphore;

    const ChunkReference& next = loc.chunk->x_neighbours[loc.forwardDirection];
    if (next.slot)
        loc.pathStep = g_movementPaths[next.chunk->type].size - 1;
    else
        loc.pathStep = 0;
    loc.chunk = next.chunk;
    loc.forwardDirection = next.slot ^ 1;

    return MoveAbility::Ok;
}

/* 18a5:00c1 */
static void moveAlongPath(Location& loc, std::int16_t distance)
{
    for (std::int16_t i = 0; i < distance; ++i) {
        if (loc.forwardDirection) {
            if (loc.pathStep < loc.chunk->maxPathStep) {
                ++loc.pathStep;
                continue;
            }
        } else {
            if (loc.pathStep > loc.chunk->minPathStep) {
                --loc.pathStep;
                continue;
            }
        }

        const ChunkReference& next = loc.chunk->x_neighbours[loc.forwardDirection];
        if (next.slot)
            loc.pathStep = g_movementPaths[next.chunk->type].size - 1;
        else
            loc.pathStep = 0;
        loc.chunk = next.chunk;
        loc.forwardDirection = next.slot ^ 1;
    }
}

/* 18a5:044c */
static std::int16_t countPassengerCarriages(const Train& train)
{
    std::int16_t res = 0;
    for (std::uint8_t i = 0; i < train.carriageCnt; ++i) {
        switch (train.carriages[i].type) {
        // TODO use enum values instead of magic values
        case 6:
        case 7:
        case 8:
        case 9:
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
    const PathStep& p = g_movementPaths[loc.chunk->type].data[loc.pathStep];
    return { static_cast<std::int16_t>(loc.chunk->x + p.dx),
             static_cast<std::int16_t>(loc.chunk->y + p.dy) };
}

/* 132d:0239 */
static void animateCollisionAndPlaySound(Position)
{
    // TODO implement
}

/* 18fa:023d */
static void createCrashMarker(Carriage&)
{
    // TODO implement
}

/* 18fa:063d */
static void showPassengerAccidentMessage(std::int16_t count)
{
    // TODO implement
}

/* 18fa:00ba */
void removeTrainFromDrawingChain(Train& train)
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

/* 19b2:0077 */
static void playTrainFinishedMelody(std::int16_t trainLen)
{
    // TODO implement
}

// TODO move to another place
/* 12ba:0003 */
void showStatusMessage(const char* msg)
{
    // TODO implement
}

/* 18a5:03a5 */
static void reverseTrain(Train& train)
{
    std::swap(train.tail, train.head);
    for (std::uint8_t i = 0; i < train.carriageCnt; ++i)
        train.carriages[i].location.forwardDirection = !train.carriages[i].location.forwardDirection;
    train.head.forwardDirection = !train.head.forwardDirection;
    train.tail.forwardDirection = !train.tail.forwardDirection;
    if (train.x_headCarriageIdx == 0)
        train.x_headCarriageIdx = train.carriageCnt - 1;
    else
        train.x_headCarriageIdx = 0;
    train.x_acceleration = 1;
}

/* 18a5:0145 */
bool moveTrain(Train& train, std::int16_t dTime)
{
    const std::int16_t speed = train.x_acceleration * dTime * 10 + train.x_speed;
    const std::int16_t maxDistance = speed >> 8;
    train.x_speed = speed;

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
            createCrashMarker(train.carriages[train.x_headCarriageIdx]);

            if (passengerCnt)
                showPassengerAccidentMessage(passengerCnt);

            spendMoney(train.carriageCnt + passengerCnt * 9);
            return true;
        }
    } else {
        const EntranceInfo& dstEntrance = g_entrances[train.carriages[0].dstEntranceIdx];
        if (train.head.chunk == dstEntrance.chunk.x_neighbours[0].chunk)
            train.x_maxSpeed = 30;
        if (train.head.chunk->type != 6) /* 6 means an entrance chunk; TODO make a constant */
            return false;
        const Rectangle& rect = train.carriages[train.carriageCnt - 1 - train.x_headCarriageIdx].rect;
        if (rect.x1 < 638 && rect.x2 > 1)
            return false;

        if (train.head.chunk == &dstEntrance.chunk ||
            train.carriages[0].type == CarriageType::Server ||
            train.carriages[0].dstEntranceIdx == blinkingTrainEntranceIdx) {
            // the train has reached the entrance
            deleteTrain(train);
            drawTrainFinishedExclamation((rect.x1 + rect.x2) / 2, (rect.y1 + rect.y2) / 2);
            playTrainFinishedMelody(train.carriageCnt);
            if (train.carriages[0].type == CarriageType::Server)
                showStatusMessage("Server went out");
            else {
                startHeaderFieldAnimation(HeaderFieldId::Trains, 1);
                startHeaderFieldAnimation(HeaderFieldId::Money, train.carriageCnt + 3);
            }
            // the original game also has this duplicated call, it looks like a bug:
            // 18a5:0323 and 18a5:038b
            drawTrainFinishedExclamation((rect.x1 + rect.x2) / 2, (rect.y1 + rect.y2) / 2);
            return false;
        }
    }
    reverseTrain(train);
    return false;
}

/* 18fa:03a8 */
static bool handleCollisions(const Carriage& c1, const Carriage& c2)
{
    // TODO implement
    return false;
}

/* 18fa:0b10 */
static void eraseCarriagesInShadowBuffer(const Carriage& c, VideoMemPtr ptr)
{
    std::int16_t widthBytes = sar<std::int16_t>((c.rect.x2 - 1), 3) - sar(c.rect.x1, 3) + 1;
    std::int16_t height = c.rect.y2 - c.rect.y1;
    if (c.next)
        eraseCarriagesInShadowBuffer(*c.next, ptr + widthBytes * height);
    drawing::copySpriteFromShadowBuffer(ptr, c.rect.x1, c.rect.y1 + 350, widthBytes, height);
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
                if (!carriage->train->x_needToMove) {
                    carriage->train->x_needToMove = true;
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

                // TODO
                // if (someLogic)
                //    needRedrawCursor = true;
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
            // TODO
            // waitVGARetrace();
            // if (needToRedrawCursor)
            //      drawCursor()

            drawing::setVideoModeR0W1();
            do {
                drawing::copyFromShadowBuffer(*curCarriageRect++);
                carriage = carriage->next;
            } while (carriage);
            drawing::setVideoModeR0W2();

            // TODO
            // if (needToRedrawCursor)
            //      drawCursor()

            drawing::setVideoModeR0W1();
            eraseCarriagesInShadowBuffer(*g_trainDrawingChains[curChain], drawing::VIDEO_MEM_SHADOW_BUFFER);
            drawing::setVideoModeR0W2();
        }
    }
    co_return;
}

} // namespace resl
