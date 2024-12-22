#include "status_bar.h"

#include <graphics/color.h>
#include <graphics/drawing.h>
#include <graphics/text.h>

#include <cstdint>
#include <cstring>

namespace resl {

/* 1d7d:0090 : 1 byte */
static std::int8_t g_footerMessageTimeout = 1;

/* 12ba:0097 */
void drawCopyright(std::int16_t yOffset)
{
    drawTextSmall(12, 336 + yOffset,
                  " * SHORTLINE * Game by Andrei Snegov * (c) DOKA 1992 Moscow * Version 1.1 *",
                  Color::Black);
}

/* 132d:013c */
void drawStatusBarWithCopyright(std::int16_t yOffset)
{
    graphics::filledRectangle(0, g_footerYPos + yOffset, 80, 16, 0xFF, Color::Gray);
    graphics::filledRectangle(0, g_footerYPos + yOffset, 80, 1, 0xFF, Color::Black);
    drawCopyright(yOffset);
}

/* 12ba:0003 */
void showStatusMessage(const char* msg, std::int16_t yOffset)
{
    g_footerMessageTimeout = 7;
    graphics::filledRectangle(0, yOffset + 335, 80, 15, 0xFF, Color::White);
    const std::int16_t msgLen = static_cast<std::int16_t>(std::strlen(msg));
    drawTextSmall((80 - msgLen) * 4, yOffset + 336, msg, Color::Black);
}

/* 12ba:005c */
void updateStatusBar()
{
    if (!g_footerMessageTimeout)
        return;

    if (--g_footerMessageTimeout == 0) {
        graphics::filledRectangle(0, 335, 80, 15, 0xFF, Color::Gray);
        drawCopyright(0);
    }
}

} // namespace resl
