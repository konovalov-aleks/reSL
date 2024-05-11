#include "move_trains.h"

#include "drawing.h"
#include "game_data.h"
#include "resources/movement_paths.h"
#include "types/header_field.h"
#include "types/position.h"

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

// TODO move to another place
/* 19de:0490 */
static void changeMoneyVolume(std::int16_t delta)
{
    // TODO implement
}

/* 18a5:042f */
static void deleteTrain(Train& train)
{
    // TODO implement
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

// TODO move to another place
/* 12c5:031b */
void startHeaderFieldAnimation(HeaderFieldId, std::int16_t delta)
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

            changeMoneyVolume(train.carriageCnt + passengerCnt * 9);
            return true;
        }
    } else {
        const EntranceInfo& dstEntrance = entrances[train.carriages[0].dstEntranceIdx];
        if (train.head.chunk == dstEntrance.chunk.x_neighbours[0].chunk)
            train.x_maxSpeed = 30;
        if (train.head.chunk->type != 6) /* 6 means an entrance chunk; TODO make a constant */
            return false;
        const Rectangle& rect = train.carriages[train.carriageCnt - train.x_headCarriageIdx].rect;
        if (rect.x1 < 638 || rect.x2 > 1)
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

} // namespace resl
