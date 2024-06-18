#include "semaphore.h"

#include "chunk.h"
#include "game_data.h"
#include "resources/rail_type_meta.h"
#include "resources/s4arr.h"
#include "resources/semaphore_glyph.h"
#include "resources/semaphore_glyph_bias.h"
#include "types/rail_info.h"
#include <graphics/color.h>
#include <graphics/glyph.h>

#include <cstdint>
#include <cstdlib>
// IWYU pragma: no_include <cmath>

namespace resl {

/* 262d:6f9c : 48 bytes */
Semaphore x_newSemaphores[4];

/* 262d:6f60 : 48 bytes */
Semaphore g_erasedSemaphores[4];

/* 262d:6f90 : 2 bytes */
std::int16_t x_newSemaphoreCount;

/* 262d:6f92 : 2 bytes */
std::int16_t g_erasedSemaphoreCount;

/* 262d:58aa : 600 bytes */
Semaphore g_semaphores[50];

/* 262d:21d6 : 2 bytes */
std::uint16_t g_semaphoreCount;

//-----------------------------------------------------------------------------

/* 19de:0426 */
static std::uint16_t roadMaskInTile(std::int16_t tileX, std::int16_t tileY)
{
    std::uint16_t tileMask = 0;
    std::uint16_t curBit = 1;
    for (const RailTypeMeta& rtm : g_railTypeMeta) {
        const std::uint16_t m = railroadTypeMasks[tileX + rtm.tileOffsetX][tileY + rtm.tileOffsetY];
        if ((1 << rtm.railType) & m)
            tileMask |= curBit;
        curBit <<= 1;
    }
    return tileMask;
}

/* 17bf:0621 */
static void createSemaphore(Chunk& c, SemaphoreType type)
{
    const s4& s4val = s4arr[c.type][static_cast<int>(type)];
    const SemaphoreGlyphBias& bias =
        g_semaphoreGlyphBias[c.type][static_cast<int>(type)];

    Semaphore& sem = g_semaphores[g_semaphoreCount];
    int dx = bias.dx;
    int dy = bias.dy;
    if ((bias.dx ^ bias.dy) >= 1)
        dx = -dx;
    else
        dy = -dy;

    sem.pixelX = c.x + (s4val.tileOffsetX - s4val.tileOffsetY) * 88 + dx;
    sem.pixelY = c.y + (s4val.tileOffsetX + s4val.tileOffsetY) * 21 + dy;
    sem.glyph = &g_semaphoreGlyphs[bias.dx > 0][bias.dy < 0];
    sem.type = type;
    sem.chunk = &c;
    sem.isRed = false;
    sem.isRightDirection = bias.dx > 0;
    c.semSlotIdByDirection[static_cast<int>(type)] = g_semaphoreCount;
    ++g_semaphoreCount;
}

/* 17bf:0754 */
static void removeSemaphore(Chunk& c, SemaphoreType type)
{
    Semaphore& s = g_semaphores[c.semSlotIdByDirection[static_cast<int>(type)]];
    Semaphore& lastSem = g_semaphores[--g_semaphoreCount];
    const std::int8_t semIdx = &s - g_semaphores;
    lastSem.chunk->semSlotIdByDirection[static_cast<int>(lastSem.type)] = semIdx;
    c.semSlotIdByDirection[static_cast<int>(type)] = -1;
    s = lastSem;
}

/* 17bf:07dd */
void createSemaphores(const RailInfo& ri)
{
    Chunk& chunk = g_chunks[ri.tileX][ri.tileY][ri.railType];
    g_erasedSemaphoreCount = 0;
    x_newSemaphoreCount = 0;
    for (int i = 0; i < 2; ++i) {
        const s4& s4data = s4arr[chunk.type][i];
        std::int16_t tileX = ri.tileX + s4data.tileOffsetX;
        std::int16_t tileY = ri.tileY + s4data.tileOffsetY;
        std::uint16_t roadMask = roadMaskInTile(tileX, tileY);
        const std::uint16_t mask = (s4data.unknown1 / 3) & 1 ? 0x1C7  // 0000000111000111
                                                             : 0xE38; // 0000111000111000

        bool create = false;
        do {
            if (roadMask & mask) {
                create = false;
                break;
            }

            roadMask = ((roadMask & 0xE38) >> 3) | (roadMask & 0x1C7);
            if (roadMask == 0x84) {
                create = true;
                break;
            }

            if (roadMask > 0x84) {
                create = !(((roadMask != 0x101) && (roadMask != 0x102)) && (roadMask != 0x104));
                break;
            }

            if (roadMask == 0x44) {
                create = true;
                break;
            }

            if (roadMask < 0x45) {
                if ((roadMask == 0x41) || (roadMask == 0x42)) {
                    create = true;
                    break;
                }
            } else if (roadMask == 0x81) {
                create = true;
                break;
            }
            create = false;
        } while (false);

        if (create) {
            for (const RailTypeMeta& rtm : g_railTypeMeta) {
                std::int16_t x = tileX + rtm.tileOffsetX;
                std::int16_t y = tileY + rtm.tileOffsetY;
                Chunk& chunk2 = g_chunks[x][y][rtm.railType];
                if (chunk2.type > 1 && ((1 << chunk2.type) & railroadTypeMasks[x][y])) {
                    createSemaphore(chunk2, rtm.semaphoreType);
                    x_newSemaphores[x_newSemaphoreCount++] =
                        g_semaphores[chunk2.semSlotIdByDirection[static_cast<int>(rtm.semaphoreType)]];
                }
            }
        } else {
            for (const RailTypeMeta& rtm : g_railTypeMeta) {
                Chunk& chunk2 =
                    g_chunks[tileX + rtm.tileOffsetX][tileY + rtm.tileOffsetY][rtm.railType];
                if (chunk2.semSlotIdByDirection[static_cast<int>(rtm.semaphoreType)] != -1) {
                    g_erasedSemaphores[g_erasedSemaphoreCount++] =
                        g_semaphores[chunk2.semSlotIdByDirection[static_cast<int>(rtm.semaphoreType)]];
                    removeSemaphore(chunk2, rtm.semaphoreType);
                }
            }
        }
    }
}

/* 137c:0135 */
void drawSemaphore(const Semaphore& s, std::int16_t yOffset)
{
    const SemaphoreGlyph& glyph = *s.glyph;
    const std::int16_t x = s.pixelX + glyph.xOffset;
    const std::int16_t y = s.pixelY + glyph.yOffset + yOffset;

    g_glyphHeight = 15;
    drawGlyphW8(glyph.glyphBg1, x, y, Color::White);
    drawGlyphW8(glyph.glyphBg2, x, y, Color::Black);

    g_glyphHeight = 4;
    drawGlyphW16(
        s.glyph->glyphLight, x + glyph.lightXOffset, y + glyph.lightYOffset,
        s.isRed ? Color::Red : Color::LightGreen);
}

/* 137c:01d3 */
void eraseSemaphore(const Semaphore& s, std::int16_t yOffset)
{
    const SemaphoreGlyph& glyph = *s.glyph;
    const std::int16_t x = s.pixelX + glyph.xOffset;
    const std::int16_t y = s.pixelY + glyph.yOffset + yOffset;

    g_glyphHeight = 15;
    drawGlyphW8(glyph.glyphBg1, x, y, Color::Green);

    g_glyphHeight = 4;
    drawGlyphW16(s.glyph->glyphLight, x + glyph.lightXOffset,
                 y + glyph.lightYOffset, Color::Green);
}

/* 19de:0414 */
void toggleSemaphore(Semaphore& s)
{
    s.isRed = !s.isRed;
}

/* 19de:038e */
Semaphore* findClosestSemaphore(std::int16_t x, std::int16_t y)
{
    Semaphore* res = nullptr;
    std::int16_t bestDistance = 60;

    for (Semaphore* s = g_semaphores + g_semaphoreCount - 1; s >= g_semaphores; --s) {
        const std::int16_t dx = std::abs(x - s->pixelX);
        const std::int16_t dy = std::abs(y - s->pixelY + 10);
        const std::int16_t dist = dx + dy * 4;
        if (dist < bestDistance) {
            res = s;
            bestDistance = dist;
        }
    }
    return res;
}

} // namespace resl
