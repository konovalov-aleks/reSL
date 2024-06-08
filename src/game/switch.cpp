#include "switch.h"

#include "game_data.h"
#include "resources/movement_paths.h"
#include "resources/rail_glyph.h"
#include "resources/rail_info.h"
#include "resources/s4arr.h"
#include "resources/semaphore_glyph_bias.h"
#include "train.h"
#include "types/chunk.h"
#include "types/rail_info.h"
#include <graphics/color.h>
#include <graphics/drawing.h>
#include <graphics/glyph.h>
#include <graphics/vga.h>
#include <system/driver/driver.h>
#include <utility/sar.h>

#include <cassert>
#include <cstdint>
#include <cstdlib>
#include <utility>

namespace resl {

/* 262d:21d4 : 2 bytes */
std::uint16_t g_nSwitches;

/* 262d:6954 : 1440 bytes */
Switch g_switches[80];

//-----------------------------------------------------------------------------

/* 19de:02ee */
static void updateSwitchPosition(Switch& s)
{
    const s4& s4obj = s4arr[s.exit.chunk->type][s.exit.slot];
    const SemaphoreGlyphBias& sb = g_semaphoreGlyphBias[s.exit.chunk->type][s.exit.slot];
    s.x = s.exit.chunk->x + (s4obj.tileOffsetX - s4obj.tileOffsetY) * 88 + sb.dx;
    s.y = s.exit.chunk->y + (s4obj.tileOffsetX + s4obj.tileOffsetY) * 21 + sb.dy;
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
void createSwitches(const RailInfo& r)
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
                s.exit.chunk = &chunk;
                s.exit.slot = ri._type1;
                s.disabledPath.chunk = &chunk2;
                s.disabledPath.slot = ri._type2;
                s.entry = c1_neighb;
                updateSwitchPosition(s);
                configChunkStepsForSwitch(s.disabledPath);
            }
        } else if (c2_neighb.chunk != specialChunkPtr) {
            Switch& s = g_switches[g_nSwitches++];
            s.x_someSwitchIndex = -1;
            s.exit.chunk = &chunk2;
            s.exit.slot = ri._type2;
            s.disabledPath.chunk = &chunk;
            s.disabledPath.slot = ri._type1;
            s.entry = c2_neighb;
            updateSwitchPosition(s);
            configChunkStepsForSwitch(s.disabledPath);

            for (Switch* s2 = g_switches; s2 < &s; ++s2) {
                if (s2->entry.chunk != &chunk2)
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
    s.exit.chunk->x_neighbours[s.exit.slot] = s.disabledPath;
    s.disabledPath.chunk->x_neighbours[s.disabledPath.slot] = s.exit;

    configChunkStepsForSwitch(s.entry);
    if (s.disabledPath.slot == 0)
        s.disabledPath.chunk->minPathStep = 0;
    else
        s.disabledPath.chunk->maxPathStep = g_movementPaths[s.disabledPath.chunk->type].size - 1;

    std::swap(s.entry, s.disabledPath);

    if (s.x_someSwitchIndex != -1)
        g_switches[s.x_someSwitchIndex].exit = s.entry;
}

/* 13d1:010f */
void drawSwitch(std::int16_t idx, bool drawToScreen)
{
    auto& vga = Driver::instance().vga();

    VideoMemPtr dstPtr = VIDEO_MEM_START_ADDR + static_cast<std::uint16_t>(-(idx + 1) * 30 - 1);
    Switch& s = g_switches[idx];
    Chunk* rail = s.entry.chunk;
    const RailGlyph* rg = railBackgrounds[rail->type].switches[s.entry.slot];

    drawing::setVideoModeR0W1();
    const std::uint8_t* glyphData = rg->glyph.data;
    for (std::uint8_t y = 0; y < rg->glyph.height; ++y) {
        std::int16_t yPos = rail->y + rg->dy + y;
        for (std::uint8_t xBytes = 0; xBytes < rg->glyph.width; ++xBytes) {
            if (*glyphData) {
                std::int16_t xPos = rail->x + rg->dx + xBytes * 8;
                VideoMemPtr srcPtr = VIDEO_MEM_START_ADDR + (yPos + 350) * VIDEO_MEM_ROW_BYTES + sar(xPos, 3);
                vga.write(dstPtr++, vga.read(srcPtr));
            }
            ++glyphData;
        }
    }
    drawing::setVideoModeR0W2();

    if (drawToScreen)
        drawGlyphAlignX8(&rg->glyph, rail->x + rg->dx, rail->y + rg->dy, Color::Black);

    drawGlyphAlignX8(&rg->glyph, rail->x + rg->dx, rail->y + rg->dy + 350, Color::Black);
}

/* 13d1:0001 */
void eraseSwitch(std::int16_t idx)
{
    auto& vga = Driver::instance().vga();

    VideoMemPtr srcPtr = VIDEO_MEM_START_ADDR + static_cast<std::uint16_t>(-(idx + 1) * 30 - 1);
    Switch& s = g_switches[idx];
    Chunk* rail = s.entry.chunk;
    const RailGlyph* rg = railBackgrounds[rail->type].switches[s.entry.slot];

    drawing::setVideoModeR0W1();
    const std::uint8_t* glyphData = rg->glyph.data;
    for (std::uint8_t y = 0; y < rg->glyph.height; ++y) {
        std::int16_t yPos = rail->y + rg->dy + y;
        for (std::uint8_t xBytes = 0; xBytes < rg->glyph.width; ++xBytes) {
            if (*glyphData) {
                const std::int16_t xPos = rail->x + rg->dx + xBytes * 8;
                const std::int16_t offset = yPos * VIDEO_MEM_ROW_BYTES + sar(xPos, 3);
                std::uint8_t data = vga.read(srcPtr++);
                vga.write(VIDEO_MEM_START_ADDR + offset, data);
                vga.write(drawing::VIDEO_MEM_SHADOW_BUFFER + offset, data);
            }
            ++glyphData;
        }
    }
    drawing::setVideoModeR0W2();
}

/* 13d1:026c */
void drawSwitchNoBackup(std::int16_t idx, std::int16_t yOffset)
{
    const Switch& s = g_switches[idx];
    const Chunk* rail = s.entry.chunk;
    const RailGlyph* rg = railBackgrounds[rail->type].switches[s.entry.slot];
    drawGlyphAlignX8(&rg->glyph, rail->x + rg->dx, rail->y + rg->dy + yOffset, Color::Black);
}

/* 19de:020b */
Switch* findClosestSwitch(std::int16_t x, std::int16_t y)
{
    Switch* res = nullptr;
    std::int16_t bestDistance = 60;

    for (Switch* s = g_switches + g_nSwitches - 1; s >= g_switches; --s) {
        const std::int16_t dx = std::abs(x - s->x);
        const std::int16_t dy = std::abs(y - s->y);
        const std::int16_t dist = dx + dy * 4;
        if (dist < bestDistance) {
            res = s;
            bestDistance = dist;
        }
    }
    return res;
}

/* 19de:0007 */
bool switchIsBusy(const Switch& s)
{
    for (const Train& t : g_trains) {
        if (t.isFreeSlot)
            continue;

        // switch is locked if a train occupies both parts of the switch
        bool entryBusy = s.entry.chunk == t.head.chunk || s.entry.chunk == t.tail.chunk;
        bool exitBusy = s.exit.chunk == t.head.chunk || s.exit.chunk == t.tail.chunk;

        for (std::uint8_t i = 0; i < t.carriageCnt; ++i) {
            const Carriage& c = t.carriages[i];
            if (s.entry.chunk == c.location.chunk)
                entryBusy = true;
            if (s.exit.chunk == c.location.chunk)
                exitBusy = true;
            if (entryBusy && exitBusy)
                return true;
        }
    }
    return false;
}

} // namespace resl
