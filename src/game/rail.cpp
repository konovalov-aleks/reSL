#include "rail.h"

#include "entrance.h"
#include "header.h"
#include "melody.h"
#include "resources/entrance_rails.h"
#include "resources/movement_paths.h"
#include "resources/rail_connection_bias.h"
#include "resources/rail_type_meta.h"
#include "train.h"
#include "types/header_field.h"
#include "types/rail_info.h"
#include "types/rectangle.h"
#include <system/random.h>

#include <array>
#include <cstdint>
#include <iterator>

namespace resl {

/* 262d:6ef4 : 4 bytes */
const RailConnection g_emptyRailConnection = { nullptr, 0 };

/* 262d:259a : 11880 bytes */
Rail g_rails[11][11][6]; // [tileX][tileY][railType]

/* 262d:6952 : 2 bytes */
std::uint16_t g_railRoadCount;

/* 262d:5b02 : 900 bytes */
RailInfo g_railRoad[150];

/* 262d:5e86 : 121 bytes */
std::uint8_t g_railroadTypeMasks[11][11];

//-----------------------------------------------------------------------------

/* 19de:0426 */
std::uint16_t roadMaskInTile(std::int16_t tileX, std::int16_t tileY)
{
    std::uint16_t tileMask = 0;
    std::uint16_t curBit = 1;
    for (const RailTypeMeta& rtm : g_railTypeMeta) {
        const std::uint16_t m = g_railroadTypeMasks[tileX + rtm.tileOffsetX][tileY + rtm.tileOffsetY];
        if ((1 << rtm.railType) & m)
            tileMask |= curBit;
        curBit <<= 1;
    }
    return tileMask;
}

/* 19de:04a9 */
bool checkRailWouldConflict(std::int16_t tileX, std::int16_t tileY,
                            std::int16_t railType)
{
    bool result = false;
    const std::uint8_t origMask = g_railroadTypeMasks[tileX][tileY];
    g_railroadTypeMasks[tileX][tileY] |= (1 << railType);

    // We also have to check that there are no conflicts with entrances that
    // have not yet been built.
    // Temporary mark the rails of all entrances as built.
    std::uint8_t origEntranceRailMasks[NormalEntranceCount];
    for (std::int16_t i = g_entranceCount; i < NormalEntranceCount; ++i) {
        const RailInfo& ri = g_entranceRails[g_entrances[i].entranceRailInfoIdx];
        origEntranceRailMasks[i] = g_railroadTypeMasks[ri.tileX][ri.tileY];
        g_railroadTypeMasks[ri.tileX][ri.tileY] |= (1 << ri.railType);
    }

    for (std::int16_t side = 0; side < 2; ++side) {
        const RailConnectionBias& rc = g_railConnectionBiases[railType][side];
        const std::uint16_t mask =
            roadMaskInTile(tileX + rc.tileOffsetX, tileY + rc.tileOffsetY);

        const std::uint16_t tripleMask = 7; // triple switch mask: 0b111
        if (((mask & tripleMask) == tripleMask) ||
            ((mask & (tripleMask << 3)) == (tripleMask << 3)) ||
            ((mask & (tripleMask << 6)) == (tripleMask << 6)) ||
            ((mask & (tripleMask << 9)) == (tripleMask << 9))) {
            result = true;
            break;
        }
    }

    // restore the railroad state
    for (std::int16_t i = g_entranceCount; i < NormalEntranceCount; ++i) {
        const RailInfo& ri = g_entranceRails[g_entrances[i].entranceRailInfoIdx];
        g_railroadTypeMasks[ri.tileX][ri.tileY] = origEntranceRailMasks[i];
    }
    g_railroadTypeMasks[tileX][tileY] = origMask;

    return result;
}

/* 19de:06a0 */
bool checkRailWouldConflictWithExistingRoad(std::int16_t tileX, std::int16_t tileY,
                                            std::int16_t railType)
{
    bool result = false;
    const std::uint8_t origMask = g_railroadTypeMasks[tileX][tileY];
    g_railroadTypeMasks[tileX][tileY] |= (1 << railType);

    for (std::int16_t side = 0; side < 2; ++side) {
        const RailConnectionBias& rc = g_railConnectionBiases[railType][side];
        const std::uint16_t mask =
            roadMaskInTile(tileX + rc.tileOffsetX, tileY + rc.tileOffsetY);

        const std::uint16_t tripleMask = 7; // triple switch mask: 0b111
        if (((mask & tripleMask) == tripleMask) ||
            ((mask & (tripleMask << 3)) == (tripleMask << 3)) ||
            ((mask & (tripleMask << 6)) == (tripleMask << 6)) ||
            ((mask & (tripleMask << 9)) == (tripleMask << 9))) {
            result = true;
            break;
        }
    }

    g_railroadTypeMasks[tileX][tileY] = origMask;
    return result;
}

/* 146b:03c7 */
bool isVisible(const Rail& r)
{
    const RailConnectionBias& rc = g_railConnectionBiases[r.type][0];
    std::int16_t chunkIdx = static_cast<std::int16_t>(&r - &g_rails[0][0][0]);
    constexpr std::int16_t nElementsPerCol =
        static_cast<std::int16_t>(std::size(g_rails[0][0]));
    constexpr std::int16_t nElementsPerRow =
        static_cast<std::int16_t>(std::size(g_rails[0]) * nElementsPerCol);
    std::int16_t x = chunkIdx / nElementsPerRow + rc.tileOffsetX;
    std::int16_t y = (chunkIdx % nElementsPerRow) / nElementsPerCol + rc.tileOffsetY;
    std::int16_t xPixPos = (x - y) * 88 + 320;
    return xPixPos > 0 && xPixPos < 640;
}

/* 19de:07d7 */
static bool railHasNoTrain(const Rail& rail)
{
    for (const Train& train : g_trains) {
        if (train.isFreeSlot || train.carriages[0].type == CarriageType::CrashedTrain)
            continue;
        if (train.head.rail == &rail || train.tail.rail == &rail)
            return false;
        for (std::uint8_t i = 0; i < train.carriageCnt; ++i) {
            if (train.carriages[i].location.rail == &rail)
                return false;
        }
    }
    return true;
}

/* 16a6:0771 */
void randomRailDamage()
{
    std::int16_t roadIdx = genRandomNumber(g_railRoadCount);
    const RailInfo& ri = g_railRoad[roadIdx];
    Rail& rail = g_rails[ri.tileX][ri.tileY][ri.railType];

    if (!railHasNoTrain(rail))
        return;

    Train* t = allocateTrainSlot();
    if (!t)
        return;

    const Path& path = g_movementPaths[ri.railType];
    t->carriageCnt = 1;
    Carriage& c = t->carriages[0];
    c.dstEntranceIdx = crashedTrainEntranceIdx;
    c.type = CarriageType::CrashedTrain;
    c.location.rail = &rail;
    c.location.pathStep = (path.size / 2) + symmetricRand(path.size / 8);
    t->tail = c.location;
    t->head = c.location;
    setEmptyRectangle(c.rect);
    t->needToRedraw = true;
    t->year = g_headers[static_cast<int>(HeaderFieldId::Year)].value;
    playRailDamagedMelody();
}

} // namespace resl
