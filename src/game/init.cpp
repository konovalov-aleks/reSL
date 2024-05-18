#include "init.h"

#include "game_data.h"
#include "resources/movement_paths.h"

namespace resl {

/* 146b:018c */
static void resetCarriages()
{
    for (int i = 0; i < 20; ++i)
        trains[i].isFreeSlot = true;
}

/* 146b:000c */
void resetGameData()
{
    g_railRoadCount = 0;
    g_nSwitches = 0;
    g_semaphoreCount = 0;
    entranceCount = 0;

    for (int x = 0; x < 11; ++x) {
        for (int y = 0; y < 11; ++y) {
            railroadTypeMasks[x][y] = 0;
            for (int type = 0; type < 6; ++type) {
                Chunk& c = g_chunks[x][y][type];
                c.x_neighbours[0] = x_unknownChunkConst;
                c.x_neighbours[1] = x_unknownChunkConst;
                c.minPathStep = 0;
                c.maxPathStep = g_movementPaths[type].size - 1;
                c.semSlotIdByDirection[0] = -1;
                c.semSlotIdByDirection[1] = -1;
                c.semSlotIdByDirection[2] = -1;
            }
        }
    }
    resetCarriages();
}

/* 146b:0153 */
static void initTrains()
{
    for (int trainIdx = 0; trainIdx < 20; ++trainIdx) {
        for (int carriageIdx = 0; carriageIdx < 5; ++carriageIdx)
            trains[trainIdx].carriages[carriageIdx].train = &trains[trainIdx];
    }
}

/* 146b:00cc */
void initGameData()
{
    for (std::int16_t x = 0; x < 11; ++x) {
        for (std::int16_t y = 0; y < 11; ++y) {
            for (std::uint8_t type = 0; type < 6; ++type) {
                Chunk& c = g_chunks[x][y][type];
                c.type = type;
                c.x = (x - y) * 88 + 320;
                c.y = (x + y) * 21 - 22;
            }
        }
    }
    initTrains();
}

} // namespace resl
