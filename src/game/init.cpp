#include "init.h"

#include "entrance.h"
#include "rail.h"
#include "resources/entrance_rails.h"
#include "resources/movement_paths.h"
#include "resources/rail_connection_bias.h"
#include "semaphore.h"
#include "static_object.h"
#include "switch.h"
#include "train.h"
#include "types/rail_info.h"
#include <graphics/color.h>
#include <system/random.h>

#include <cassert>
#include <cstdint>
#include <cstdlib>
#include <iterator>

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
        const std::int16_t rIdx = genRandomNumber(std::size(g_entranceRails));
        entrance.entranceRailInfoIdx = rIdx;
        const RailInfo& ri = g_entranceRails[entrance.entranceRailInfoIdx];
        for (std::int16_t j = 0; j < i; ++j) {
            const std::int16_t r2Idx = g_entrances[j].entranceRailInfoIdx;
            const RailInfo& ri2 = g_entranceRails[r2Idx];
            if (i == 1) {
                if (std::abs(rIdx - r2Idx) < 23)
                    break;
            } else {
                std::int16_t distX =
                    std::abs((ri.tileX - ri.tileY) * 88 + 320 - (ri2.tileX - ri2.tileY) * 88 + 320);
                if (distX < 320 && std::abs(rIdx - r2Idx) < 10)
                    break;
            }
        }
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
        obj.type = genRandomNumber(4);
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
}

} // namespace resl
