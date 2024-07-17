#include "switch.h"

#include "rail.h"
#include "resources/movement_paths.h"
#include "resources/rail_connection_bias.h"
#include "resources/rail_connection_rule.h"
#include "resources/rail_glyph.h"
#include "resources/semaphore_glyph_bias.h"
#include "train.h"
#include "types/rail_info.h"
#include <graphics/color.h>
#include <graphics/drawing.h>
#include <graphics/glyph.h>
#include <graphics/vga.h>
#include <system/driver/driver.h>
#include <utility/sar.h>

#include <array>
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
    const RailConnectionBias& rc = g_railConnectionBiases[s.exit.rail->type][s.exit.slot];
    const SemaphoreGlyphBias& sb = g_semaphoreGlyphBiases[s.exit.rail->type][s.exit.slot];
    s.x = s.exit.rail->x + (rc.tileOffsetX - rc.tileOffsetY) * 88 + sb.dx;
    s.y = s.exit.rail->y + (rc.tileOffsetX + rc.tileOffsetY) * 21 + sb.dy;
}

/* 19de:00b6 */
static void configChunkStepsForSwitch(RailConnection r)
{
    r.rail->connections[r.slot].rail = g_disabledSwitchPath;
    if (r.slot == 0)
        r.rail->minPathStep = 82;
    else
        r.rail->maxPathStep = g_movementPaths[r.rail->type].size - 83;
}

/* 1ad3:000c */
void createSwitches(const RailInfo& r)
{
    assert(r.tileX < 11);
    assert(r.tileY < 11);
    assert(r.railType < 6);
    Rail& rail = g_rails[r.tileX][r.tileY][r.railType];
    g_railroadTypeMasks[r.tileX][r.tileY] |= (1 << r.railType);

    for (std::uint16_t i = 0; i < 6; ++i) {
        const RailConnectionRule& ri = g_railConnectionRules[r.railType][i];
        const std::int16_t tileX = r.tileX + ri.tileDX;
        const std::int16_t tileY = r.tileY + ri.tileDY;
        if (!(g_railroadTypeMasks[tileX][tileY] & (1 << ri.railType)))
            continue;

        Rail& rail2 = g_rails[tileX][tileY][ri.railType];
        RailConnection& rc1 = rail.connections[ri.slot1];
        RailConnection& rc2 = rail2.connections[ri.slot2];
        if (!rc2.rail) {
            if (!rc1.rail) {
                // no adjacent roads
                rc1.rail = &rail2;
                rc1.slot = ri.slot2;
                rc2.rail = &rail;
                rc2.slot = ri.slot1;
            } else if (rc1.rail != g_disabledSwitchPath) {
                Switch& s = g_switches[g_nSwitches++];
                s.adjucentSwitchIdx = -1;
                s.exit.rail = &rail;
                s.exit.slot = ri.slot1;
                s.disabledPath.rail = &rail2;
                s.disabledPath.slot = ri.slot2;
                s.entry = rc1;
                updateSwitchPosition(s);
                configChunkStepsForSwitch(s.disabledPath);
            }
        } else if (rc2.rail != g_disabledSwitchPath) {
            Switch& s = g_switches[g_nSwitches++];
            s.adjucentSwitchIdx = -1;
            s.exit.rail = &rail2;
            s.exit.slot = ri.slot2;
            s.disabledPath.rail = &rail;
            s.disabledPath.slot = ri.slot1;
            s.entry = rc2;
            updateSwitchPosition(s);
            configChunkStepsForSwitch(s.disabledPath);

            for (Switch* s2 = g_switches; s2 < &s; ++s2) {
                // Handle X-shaped crossings.
                // In this case, we switches are connected each other directly.
                if (s2->entry.rail == &rail2 && s2->entry.slot == ri.slot2) {
                    s.adjucentSwitchIdx = s2 - g_switches;
                    s2->adjucentSwitchIdx = &s - g_switches;
                    break;
                }
            }
        }
    }
}

/* 19de:00ec */
void toggleSwitch(Switch& s)
{
    s.exit.rail->connections[s.exit.slot] = s.disabledPath;
    s.disabledPath.rail->connections[s.disabledPath.slot] = s.exit;

    configChunkStepsForSwitch(s.entry);
    if (s.disabledPath.slot == 0)
        s.disabledPath.rail->minPathStep = 0;
    else
        s.disabledPath.rail->maxPathStep = g_movementPaths[s.disabledPath.rail->type].size - 1;

    std::swap(s.entry, s.disabledPath);

    if (s.adjucentSwitchIdx != -1)
        g_switches[s.adjucentSwitchIdx].exit = s.entry;
}

/* 13d1:010f */
void drawSwitch(std::int16_t idx, bool drawToScreen)
{
    auto& vga = Driver::instance().vga();

    vga::VideoMemPtr dstPtr =
        vga::VIDEO_MEM_START_ADDR + static_cast<std::uint16_t>(-(idx + 1) * 30 - 1);
    Switch& s = g_switches[idx];
    Rail* rail = s.entry.rail;
    const RailGlyph* rg = railBackgrounds[rail->type].switches[s.entry.slot];

    vga::setVideoModeR0W1();
    const std::uint8_t* glyphData = rg->glyph.data;
    for (std::uint8_t y = 0; y < rg->glyph.height; ++y) {
        std::int16_t yPos = rail->y + rg->dy + y;
        for (std::uint8_t xBytes = 0; xBytes < rg->glyph.width; ++xBytes) {
            if (*glyphData) {
                std::int16_t xPos = rail->x + rg->dx + xBytes * 8;
                vga::VideoMemPtr srcPtr =
                    vga::VIDEO_MEM_START_ADDR + (yPos + 350) * vga::VIDEO_MEM_ROW_BYTES + sar(xPos, 3);
                vga.write(dstPtr++, vga.read(srcPtr));
            }
            ++glyphData;
        }
    }
    vga::setVideoModeR0W2();

    if (drawToScreen)
        drawGlyphAlignX8(&rg->glyph, rail->x + rg->dx, rail->y + rg->dy, Color::Black);

    drawGlyphAlignX8(&rg->glyph, rail->x + rg->dx, rail->y + rg->dy + 350, Color::Black);
}

/* 13d1:0001 */
void eraseSwitch(std::int16_t idx)
{
    auto& vga = Driver::instance().vga();

    vga::VideoMemPtr srcPtr =
        vga::VIDEO_MEM_START_ADDR + static_cast<std::uint16_t>(-(idx + 1) * 30 - 1);
    Switch& s = g_switches[idx];
    Rail* rail = s.entry.rail;
    const RailGlyph* rg = railBackgrounds[rail->type].switches[s.entry.slot];

    vga::setVideoModeR0W1();
    const std::uint8_t* glyphData = rg->glyph.data;
    for (std::uint8_t y = 0; y < rg->glyph.height; ++y) {
        std::int16_t yPos = rail->y + rg->dy + y;
        for (std::uint8_t xBytes = 0; xBytes < rg->glyph.width; ++xBytes) {
            if (*glyphData) {
                const std::int16_t xPos = rail->x + rg->dx + xBytes * 8;
                const std::int16_t offset = yPos * vga::VIDEO_MEM_ROW_BYTES + sar(xPos, 3);
                std::uint8_t data = vga.read(srcPtr++);
                vga.write(vga::VIDEO_MEM_START_ADDR + offset, data);
                vga.write(graphics::VIDEO_MEM_SHADOW_BUFFER + offset, data);
            }
            ++glyphData;
        }
    }
    vga::setVideoModeR0W2();
}

/* 13d1:026c */
void drawSwitchNoBackup(std::int16_t idx, std::int16_t yOffset)
{
    const Switch& s = g_switches[idx];
    const Rail* rail = s.entry.rail;
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
        bool entryBusy = s.entry.rail == t.head.rail || s.entry.rail == t.tail.rail;
        bool exitBusy = s.exit.rail == t.head.rail || s.exit.rail == t.tail.rail;

        for (std::uint8_t i = 0; i < t.carriageCnt; ++i) {
            const Carriage& c = t.carriages[i];
            if (s.entry.rail == c.location.rail)
                entryBusy = true;
            if (s.exit.rail == c.location.rail)
                exitBusy = true;
            if (entryBusy && exitBusy)
                return true;
        }
    }
    return false;
}

} // namespace resl
