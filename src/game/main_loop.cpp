#include "main_loop.h"

#include "entrance.h"
#include "header.h"
#include "mouse/construction_mode.h"
#include "mouse/mouse_mode.h"
#include "mouse/mouse_state.h"
#include "resources/entrance_rails.h"
#include "status_bar.h"
#include "train.h"
#include "types/header_field.h"
#include "types/rail_info.h"
#include <graphics/color.h>
#include <graphics/vga.h>
#include <tasks/task.h>

#include <cstdint>
#include <cstdio>

namespace resl {

// Time elapsed since the beginning of the current game year.
// One game year corresponds to the value 2000 of this counter.
/* 262d:6f50 : 2 bytes */
static std::int16_t g_gameTime = 0;

/* 16a6:0750 */
static std::int16_t incrementGameTime(std::int16_t delta)
{
    g_gameTime += delta;
    if (g_gameTime < 2000)
        return 0;

    g_gameTime -= 2000;
    return g_headers[static_cast<int>(HeaderFieldId::Year)].value;
}

/* 16a6:0001 */
Task taskGameMainLoop()
{
    g_gameTime = 0;

    // The original game uses uint8 here - VGA color code.
    // They alternate between 0 and 0x3F.
    std::uint32_t blinkingColor = 0xFFFFFF;

    for (;;) {
        co_await sleep(50);

        accelerateTrains(5);
        vga::waitVerticalRetrace();

        blinkingColor ^= 0xFFFFFF;
        vga::setPaletteItem(Color::BWBlinking, blinkingColor);

        tryRunWaitingTrains();

        if (mouse::g_state.mode == &mouse::g_modeConstruction)
            mouse::g_state.mode->drawFn();

        std::int16_t newYear = incrementGameTime(100);
        if (newYear) {
            startHeaderFieldAnimation(HeaderFieldId::Year, 1);
            if (newYear & 1)
                startHeaderFieldAnimation(HeaderFieldId::Money, -1);

            const std::int16_t waitingTrains = waitingTrainsCount();
            if (waitingTrains) {
                char buf[60];
                std::snprintf(buf, sizeof(buf), "%d,000 penalty for waiting trains",
                              static_cast<int>(waitingTrains));
                showStatusMessage(buf);
                startHeaderFieldAnimation(HeaderFieldId::Money, -waitingTrains);
            }

            switch (g_headers[static_cast<int>(HeaderFieldId::Year)].value) {
            case 1800:
                if (g_entranceCount == 1)
                    spawnNewEntrance(g_entranceRails[g_entrances[1].entranceRailInfoIdx]);
                break;
            case 1840:
                if (g_entranceCount == 2)
                    spawnNewEntrance(g_entranceRails[g_entrances[2].entranceRailInfoIdx]);
                break;
            case 1880:
                if (g_entranceCount == 3)
                    spawnNewEntrance(g_entranceRails[g_entrances[3].entranceRailInfoIdx]);
                break;
            case 1920:
                if (g_entranceCount == 4)
                    spawnNewEntrance(g_entranceRails[g_entrances[4].entranceRailInfoIdx]);
                break;
            case 1960:
                if (g_entranceCount == 5)
                    spawnNewEntrance(g_entranceRails[g_entrances[5].entranceRailInfoIdx]);
                break;
            case 2000:
                // TODO
                // show happy 2000 banner
                // stop game
                break;
            };
        }
    }
}

} // namespace resl
