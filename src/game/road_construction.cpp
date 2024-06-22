#include "road_construction.h"

#include "drawing.h"
#include "header.h"
#include "impasse.h"
#include "melody.h"
#include "mouse/mouse_mode.h"
#include "mouse/mouse_state.h"
#include "rail.h"
#include "resources/rail_connection_bias.h"
#include "resources/rail_type_meta.h"
#include "semaphore.h"
#include "static_object.h"
#include "status_bar.h"
#include "switch.h"
#include "types/rail_info.h"
#include <graphics/color.h>
#include <system/random.h>
#include <system/sound.h>
#include <tasks/message_queue.h>
#include <tasks/task.h>

#include <cstdio>

namespace resl {

/* 1d7d:1c96 */
MessageQueue<RailInfo> g_railConstructionMsgQueue;

//-----------------------------------------------------------------------------

// When constructing new rails, the gray background may overlap existing rails
// at the joints (erase their black lines).
// This function redraws adjacent rails to avoid this problem.
/* 17bf:0aa0 */
static void redrawAdjacentRails(const RailInfo& ri)
{
    for (std::int16_t side = 0; side < 2; ++side) {
        const std::int16_t x =
            g_railConnectionBiases[ri.railType][side].tileOffsetX + ri.tileX;
        const std::int16_t y =
            g_railConnectionBiases[ri.railType][side].tileOffsetY + ri.tileY;

        for (const RailTypeMeta& rt : g_railTypeMeta) {
            const std::int16_t tileX = x + rt.tileOffsetX;
            const std::int16_t tileY = y + rt.tileOffsetX;
            if ((1 << rt.railType) & g_railroadTypeMasks[tileX][tileY])
                drawRail(tileX, tileY, rt.railType, Color::Black, 350);
        }
    }
}

/* 17bf:000d */
Task taskRoadConstruction()
{
    for (;;) {
        const RailInfo ri = co_await g_railConstructionMsgQueue.pop();

        if ((1 << ri.railType) & g_railroadTypeMasks[ri.tileX][ri.tileY]) {
            showStatusMessage("ALREADY BUILT");
            playErrorMelody();
            continue;
        }

        if (checkRailWouldConflict(ri.tileX, ri.tileY, ri.railType)) {
            showStatusMessage("Can\'t construct triple switch");
            playErrorMelody();
            continue;
        }

        const Rail& rail = g_rails[ri.tileX][ri.tileY][ri.railType];
        std::int16_t penalty = ri.railType < 2 ? 1 : 2;
        if (penalty > g_headers[static_cast<int>(HeaderFieldId::Money)].value) {
            showStatusMessage("Not enough money to build");
            playErrorMelody();

            mouse::g_state.mode->clearFn();
            drawRail(ri.tileX, ri.tileY, ri.railType, Color::Green, 350);
            mouse::g_state.mode->drawFn();

            scheduleRailRedrawing(rail);
            clampRectToGameFieldBoundaries(g_areaToRedraw);
            drawFieldBackground(350);
            redrawScreenArea();
            continue;
        }

        spendMoney(penalty);

        g_areaToRedraw.y1 = 32000;
        g_areaToRedraw.x1 = 32000;
        g_areaToRedraw.y2 = 0;
        g_areaToRedraw.x2 = 0;

        destroyStaticObjectsForRailConstruction(rail);
        penalty = g_cuttingDownStaticObjectsByKind[StaticObjectKind::House] +
            g_cuttingDownStaticObjectsByKind[StaticObjectKind::Tree];
        if (penalty) {
            showStatusMessage("Cutting down the tree(s) or house(s)");
            for (std::int16_t i = 0; i < penalty; ++i) {
                if (g_soundEnabled)
                    sound(genRandomNumber(40) + 30);
                co_await sleep(4);
                nosound();
                co_await sleep(18);
            }
            char msg[80];
            std::snprintf(msg, sizeof(msg), "... and paying $%d,000 FINE",
                          static_cast<int>(penalty));
            showStatusMessage(msg);
            spendMoney(penalty);
            drawStaticObjects(350);
            clampRectToGameFieldBoundaries(g_areaToRedraw);
            redrawScreenArea();
            beepSound(1);
        }

        co_await sleep(100);
        beepSound(2);
        eraseImpasse(rail, 350);
        drawRail(ri.tileX, ri.tileY, ri.railType, Color::Gray, 350);
        drawRailBg1(ri.tileX, ri.tileY, ri.railType, Color::DarkGray, 350);
        scheduleRailRedrawing(rail);
        clampRectToGameFieldBoundaries(g_areaToRedraw);
        redrawScreenArea();

        co_await sleep(100);
        beepSound(3);
        drawRailBg2(ri.tileX, ri.tileY, ri.railType, Color::Gray, 350);
        drawRail(ri.tileX, ri.tileY, ri.railType, Color::Gray, 350);
        redrawAdjacentRails(ri);
        redrawScreenArea();

        co_await sleep(100);
        beepSound(4);
        createSwitches(ri);
        g_railRoad[g_railRoadCount++] = ri;
        createSemaphores(ri);
        for (std::int16_t i = 0; i < g_erasedSemaphoreCount; ++i)
            eraseSemaphore(g_erasedSemaphores[i], 350);
        drawFieldBackground(0x15e);
        redrawScreenArea();
    }
}

} // namespace resl
