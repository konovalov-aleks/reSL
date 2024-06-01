#include "semaphore.h"

#include "game_data.h"
#include "resources/rail_type_meta.h"
#include "resources/s4arr.h"
#include "resources/semaphore_glyph.h"
#include "resources/semaphore_glyph_bias.h"
#include "types/chunk.h"
#include "types/rail_info.h"
#include "types/semaphore.h"

#include <cstdint>

namespace resl {

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
void createSemaphores(RailInfo& ri)
{
    Chunk& chunk = g_chunks[ri.tileX][ri.tileY][ri.railType];
    x_erasedSemaphoreCount = 0;
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
                    x_erasedSemaphores[x_erasedSemaphoreCount++] =
                        g_semaphores[chunk2.semSlotIdByDirection[static_cast<int>(rtm.semaphoreType)]];
                    removeSemaphore(chunk2, rtm.semaphoreType);
                }
            }
        }
    }
}

} // namespace resl
