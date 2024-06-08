#include "entrance.h"

#include "train.h"
#include "types/chunk.h"

#include <cstdint>
#include <cstdlib>

namespace resl {

/* 262d:5f00 : 2 bytes */
std::int16_t g_entranceCount;

/* g_entrances is writable, but it has an initial state.
   So, the declaration of g_entrances is generated and located in the resources folder
*/
/* 1d7d:01fc : 198 bytes */
// EntranceInfo g_entrances[9];

//-----------------------------------------------------------------------------

/* 19de:028b */
Entrance* findClosestEntrance(std::int16_t x, std::int16_t y)
{
    Entrance* result = nullptr;
    std::int16_t bestDistance = 200;
    for (std::int16_t i = 0; i < g_entranceCount; ++i) {
        Entrance& entrance = g_entrances[i];
        const std::int16_t dx = std::abs(x - entrance.chunk.x);
        const std::int16_t dy = std::abs(y - entrance.chunk.y);
        const std::int16_t dist = dx + dy * 4;
        if (dist < bestDistance) {
            bestDistance = dist;
            result = &entrance;
        }
    }
    return result;
}

/* 1a65:039d */
bool entranceIsFree(std::int16_t entranceIdx)
{
    const Entrance& entrance = g_entrances[entranceIdx];
    for (const Train& train : g_trains) {
        if (train.isFreeSlot || train.carriages[0].type == CarriageType::CrashedTrain)
            continue;
        if (train.head.chunk == &entrance.chunk ||
            train.head.chunk == entrance.chunk.x_neighbours[0].chunk ||
            train.tail.chunk == &entrance.chunk ||
            train.tail.chunk == entrance.chunk.x_neighbours[0].chunk)
            return false;
    }
    return true;
}

} // namespace resl
