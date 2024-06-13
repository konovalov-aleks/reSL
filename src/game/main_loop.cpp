#include "main_loop.h"

#include "header.h"
#include "status_bar.h"
#include "train.h"
#include "types/header_field.h"
#include <graphics/color.h>
#include <graphics/drawing.h>
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

        blinkingColor ^= 0xFFFFFF;
        drawing::setPaletteItem(Color::BWBlinking, blinkingColor);

        tryRunWaitingTrains();

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
        }
    }
}

} // namespace resl
