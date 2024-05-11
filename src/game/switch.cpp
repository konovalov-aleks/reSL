#include "switch.h"

#include "game_data.h"
#include "resources/movement_paths.h"
#include "resources/rail_info.h"
#include "resources/s4arr.h"
#include "resources/semaphore_glyph_bias.h"

#include <cassert>
#include <utility>

namespace resl {

/* 19de:02ee */
static void updateSwitchPosition(Switch& s)
{
    const s4& s4obj = s4arr[s.c1.chunk->type][s.c1.slot];
    const SemaphoreGlyphBias& sb = g_semaphoreGlyphBias[s.c1.chunk->type][s.c1.slot];
    s.x = s.c1.chunk->x + (s4obj.tileOffsetX - s4obj.tileOffsetY) * 88 + sb.dx;
    s.y = s.c1.chunk->y + (s4obj.tileOffsetX - s4obj.tileOffsetY) * 21 + sb.dy;
}

/* 19de:00b6 */
static void configChunkStepsForSwitch(ChunkReference r)
{
    r.chunk->x_neighbours[r.slot].chunk = specialChunkPtr;
    if (r.slot == 0)
        r.chunk->minPathStep = 82;
    else
        r.chunk->maxPathStep = g_movementPaths[r.chunk->type].size - 83;
}

/* 1ad3:000c */
void createSwitches(RailInfo& r)
{
    assert(r.tileX < 11);
    assert(r.tileY < 11);
    assert(r.railType < 6);
    Chunk& chunk = g_chunks[r.tileX][r.tileY][r.railType];
    railroadTypeMasks[r.tileX][r.tileY] |= (1 << r.railType);

    for (std::uint16_t i = 0; i < 6; ++i) {
        const RailInfo2& ri = x_railInfo[r.railType][i];
        const std::int16_t tileX = r.tileX + ri.tileDX;
        const std::int16_t tileY = r.tileY + ri.tileDY;
        if (!(railroadTypeMasks[tileX][tileY] & (1 << ri.railType)))
            continue;

        Chunk& chunk2 = g_chunks[tileX][tileY][ri.railType];
        ChunkReference& c1_neighb = chunk.x_neighbours[ri._type1];
        ChunkReference& c2_neighb = chunk2.x_neighbours[ri._type2];
        if (!c2_neighb.chunk) {
            if (!c1_neighb.chunk) {
                // no adjacent roads
                c1_neighb.chunk = &chunk2;
                c1_neighb.slot = ri._type2;
                c2_neighb.chunk = &chunk;
                c2_neighb.slot = ri._type1;
            } else if (c1_neighb.chunk != specialChunkPtr) {
                Switch& s = g_switches[g_nSwitches++];
                s.x_someSwitchIndex = -1;
                s.c1.chunk = &chunk;
                s.c1.slot = ri._type1;
                s.c3.chunk = &chunk2;
                s.c3.slot = ri._type2;
                s.curChunk = c1_neighb;
                updateSwitchPosition(s);
                configChunkStepsForSwitch(s.c3);
            }
        } else if (c2_neighb.chunk != specialChunkPtr) {
            Switch& s = g_switches[g_nSwitches++];
            s.x_someSwitchIndex = -1;
            s.c1.chunk = &chunk2;
            s.c1.slot = ri._type2;
            s.c3.chunk = &chunk;
            s.c3.slot = ri._type1;
            s.curChunk = c2_neighb;
            updateSwitchPosition(s);
            configChunkStepsForSwitch(s.c3);

            for (Switch* s2 = g_switches; s2 < &s; ++s2) {
                if (s2->curChunk.chunk != &chunk2)
                    continue;

                s.x_someSwitchIndex = s2 - g_switches;
                s2->x_someSwitchIndex = &s - g_switches;
                break;
            }
        }
    }
}

/* 19de:00ec */
void toggleSwitch(Switch& s)
{
    s.c1.chunk->x_neighbours[s.c1.slot] = s.c3;
    s.c3.chunk->x_neighbours[s.c3.slot] = s.c1;

    configChunkStepsForSwitch(s.curChunk);
    if (s.c3.slot == 0)
        s.c3.chunk->minPathStep = 0;
    else
        s.c3.chunk->maxPathStep = g_movementPaths[s.c3.chunk->type].size - 1;

    std::swap(s.curChunk, s.c3);

    if (s.x_someSwitchIndex != -1)
        g_switches[s.x_someSwitchIndex].c1 = s.curChunk;
}

} // namespace resl
