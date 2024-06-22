#include "init.h"

#include "entrance.h"
#include "rail.h"
#include "resources/movement_paths.h"
#include "semaphore.h"
#include "switch.h"
#include "train.h"

#include <cstdint>

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

} // namespace resl
