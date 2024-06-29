#include "init.h"

#include "entrance.h"
#include "rail.h"
#include "resources/entrance_rails.h"
#include "resources/movement_paths.h"
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
