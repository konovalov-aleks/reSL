#include "init.h"

#include "entrance.h"
#include "header.h"
#include "move_trains.h"
#include "rail.h"
#include "rail_info.h"
#include "resources/entrance_rails.h"
#include "resources/movement_paths.h"
#include "resources/rail_connection_bias.h"
#include "road_construction.h"
#include "semaphore.h"
#include "static_object.h"
#include "switch.h"
#include "train.h"
#include <graphics/color.h>
#include <system/random.h>
#include <tasks/message_queue.h>
#include <tasks/task.h>

#include <cassert>
#include <cstdint>
#include <cstdlib>
#include <iterator>

#ifndef NDEBUG
#   include <algorithm>
#endif // !NDEBUG

namespace resl {

/* 146b:000c */
void resetGameData()
{
    g_railRoadCount = 0;
    g_nSwitches = 0;
    g_semaphoreCount = 0;
    g_entranceCount = 0;

    for (int x = 0; x < 11; ++x) {
        for (int y = 0; y < 11; ++y) {
            g_railroadTypeMasks[x][y] = 0;
            for (int type = 0; type < 6; ++type) {
                Rail& r = g_rails[x][y][type];
                r.connections[0] = g_emptyRailConnection;
                r.connections[1] = g_emptyRailConnection;
                r.minPathStep = 0;
                r.maxPathStep = g_movementPaths[type].size - 1;
                r.semSlotIdByDirection[0] = -1;
                r.semSlotIdByDirection[1] = -1;
                r.semSlotIdByDirection[2] = -1;
            }
        }
    }
    resetCarriages();
}

/* 146b:00cc */
void initGameData()
{
    for (std::int16_t x = 0; x < 11; ++x) {
        for (std::int16_t y = 0; y < 11; ++y) {
            for (std::uint8_t type = 0; type < 6; ++type) {
                Rail& r = g_rails[x][y][type];
                r.type = type;
                r.x = (x - y) * 88 + 320;
                r.y = (x + y) * 21 - 22;
            }
        }
    }
    initTrains();
}

/* 1530:04f5 */
static bool validateHousePosition(const StaticObject& house, std::int16_t i)
{
    const std::int16_t x = house.x + 8;

    // Houses can only be placed at the edge of the screen;
    // valid X coordinate ranges are: [0; 50] and [590; 640]
    if ((x < 0 || x > 50) && (x < 590 || x > 640))
        return false;

    // Houses can't be located too close to each other
    for (const StaticObject* house2 = &house - 1; i > 0; --i, --house2) {
        std::int16_t dy = std::abs(house.y - house2->y);
        std::int16_t dx = std::abs(house.x - house2->x);
        if (dy < 10 && dx < 10)
            return false;
    }
    return true;
}

/* 1530:0351 */
static void generateEntranceHouses(std::int16_t entranceIdx)
{
    // number of houses created next to each entrance
    constexpr std::int16_t housesPerEntrance = 5;

    const RailInfo& ri = g_entranceRails[g_entrances[entranceIdx].entranceRailInfoIdx];
    for (std::int16_t i = 0; i < housesPerEntrance; ++i) {
        std::int16_t objIdx = i + entranceIdx * housesPerEntrance;
        StaticObject& obj = g_staticObjects[objIdx];
        obj.kind = StaticObjectKind::BuildingHouse;
        obj.type = genRandomNumber(4) + 1;
        obj.color = g_entrances[entranceIdx].bgColor;
        if (entranceIdx == 0 || entranceIdx == 1)
            obj.creationYear = 0;
        else
            obj.creationYear = (entranceIdx - 2) * 40 + i * 8 + 5;

        do {
            std::int16_t stepIdx = genRandomNumber(g_movementPaths[ri.railType].size);
            const PathStep& step = g_movementPaths[ri.railType].data[stepIdx];
            obj.x = (ri.tileX - ri.tileY) * 88 + step.dx + 320;
            obj.y = (ri.tileX + ri.tileY) * 21 + step.dy - 22;

            std::int16_t rnd = genRandomNumber(4);
            obj.y += (rand() & 1) ? -23 - rnd : 5 + rnd;
        } while (!validateHousePosition(obj, i));
    }
}

/* 146b:01a7 */
static void generateEntrances()
{
    for (std::int16_t i = 0; i < NormalEntranceCount; ++i) {
        Entrance& entrance = g_entrances[i];

        /*** BUGFIX ***

        ShortLine v1.1 has a bug: if the previous game session was stopped
        when the first entrance has waiting trains (the yellow dispatcher is
        showing the flag), then when starting a new game the program will freeze.
        The reason is that they don't reset 'entrance.waitingTrains' variable =>
        it starts executing 'tryRunWaitingTrains' function (16a6:08bb). This
        function in a loop tries to generate a pair of random existing
        entrances, but at the beginning there is only one entrance
        => the loop become infinite

        The following line fixes the problem:
        */
        entrance.waitingTrainsCount = 0;

        bool suits = false;
        while (!suits) {
            const std::int16_t r1Idx =
                genRandomNumber(static_cast<std::int16_t>(std::size(g_entranceRails)));
            entrance.entranceRailInfoIdx = static_cast<std::uint8_t>(r1Idx);
            const RailInfo& ri1 = g_entranceRails[entrance.entranceRailInfoIdx];

            suits = true;
            for (std::int16_t j = 0; j < i; ++j) {
                const std::int16_t r2Idx = g_entrances[j].entranceRailInfoIdx;
                const RailInfo& ri2 = g_entranceRails[r2Idx];
                if (i == 1) {
                    if (std::abs(r1Idx - r2Idx) < 23) {
                        suits = false;
                        break;
                    }
                } else {
                    const std::int16_t y1 = (ri1.tileX - ri1.tileY) * 88 + 320;
                    const std::int16_t y2 = (ri2.tileX - ri2.tileY) * 88 + 320;
                    const std::int16_t distY = std::abs(y1 - y2);
                    if (distY < 320 && std::abs(r1Idx - r2Idx) < 10) {
                        suits = false;
                        break;
                    }
                }
            }
        }

        const RailInfo& ri = g_entranceRails[entrance.entranceRailInfoIdx];
        Rail& rail = g_rails[ri.tileX][ri.tileY][ri.railType];
        bool visible = isVisible(rail);
        Rail& entranceRail = g_entrances[i].rail;
        rail.connections[visible].rail = &entranceRail;

        entranceRail.connections[0].slot = visible;
        entranceRail.connections[0].rail = &rail;
        entranceRail.maxPathStep =
            g_movementPaths[g_innerEntranceRailType].size - 1;
        entranceRail.type = g_innerEntranceRailType;

        const RailConnectionBias& rcb =
            g_railConnectionBiases[rail.type][visible];

        const std::int16_t tileX = ri.tileX + rcb.tileOffsetX;
        const std::int16_t tileY = ri.tileY + rcb.tileOffsetY;
        entranceRail.x = (tileX - tileY) * 88 + 320;
        entranceRail.y = (tileX + tileY) * 21 - 22;

        generateEntranceHouses(i);
    }
}

/* 1530:010f */
inline bool isInsideField(std::int16_t x, std::int16_t y)
{
    return y > 47 && y + 16 < 334 && x + 8 >= 0 && x + 8 < 640;
}

/* 1530:0002 */
static int compareStaticObjByY(const void* a, const void* b)
{
    assert(a);
    assert(b);
    const StaticObject& sa = *static_cast<const StaticObject*>(a);
    const StaticObject& sb = *static_cast<const StaticObject*>(b);
    return sa.y < sb.y ? -1 : 1;
}

/* 1530:001f */
static void generateForest()
{
    std::int16_t x = genRandomNumber(624);
    std::int16_t y = genRandomNumber(270) + 48;

    for (std::size_t i = 30; i < std::size(g_staticObjects); ++i) {
        StaticObject& obj = g_staticObjects[i];
        obj.kind = StaticObjectKind::Tree;
        obj.type = static_cast<std::uint8_t>(genRandomNumber(4));
        obj.x = x;
        obj.y = y;
        obj.color = rand() & 1 ? Color::Brown : Color::DarkGreen;

        x += symmetricRand(25);
        y += symmetricRand(25) / 4;
        if ((rand() & 0xFFFF) < 2600 || !isInsideField(x, y)) {
            x = genRandomNumber(624);
            y = genRandomNumber(271) + 47;
        }
    }
    std::qsort(static_cast<void*>(g_staticObjects),
               std::size(g_staticObjects), sizeof(StaticObject),
               compareStaticObjByY);
}

/* 16a6:0963 */
void createNewWorld()
{
    resetGameData();
    generateEntrances();
    generateForest();

#ifndef NDEBUG
    std::uint8_t data[NormalEntranceCount];
    for (std::int16_t i = g_entranceCount; i < NormalEntranceCount; ++i)
        data[i] = g_entrances[i].entranceRailInfoIdx;
    std::ranges::sort(data);
    assert(std::ranges::adjacent_find(data) == std::end(data));
#endif
}

/* 16a6:0973 */
void resetTasks()
{
    // The original game uses different coroutine implementation =>
    // this function is different there. But the common algorithm
    // is the same:
    // * clear message queues
    // * restart all tasks
    g_railConstructionMsgQueue.clear();
    stopTask(g_taskRoadConstruction);
    g_taskRoadConstruction = addTask(taskRoadConstruction());

    g_headerAnimationTaskQueue.clear();
    stopTask(g_taskHeaderFieldAnimation);
    g_taskHeaderFieldAnimation = addTask(taskHeaderFieldAnimation());

    stopTask(g_taskMoveAndRedrawTrains);
    g_taskMoveAndRedrawTrains = addTask(taskMoveAndRedrawTrains());

    // TODO
    //    g_mouseMsgQueue.clear();
}

} // namespace resl
