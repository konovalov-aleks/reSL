#include "status_bar.h"

#include <graphics/color.h>
#include <graphics/drawing.h>
#include <graphics/text.h>

#include <cstdint>
#include <iostream>

namespace resl {

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
    drawing::filledRectangle(0, g_footerYPos + yOffset, 80, 16, 0xFF, Color::Gray);
    drawing::filledRectangle(0, g_footerYPos + yOffset, 80, 1, 0xFF, Color::Black);
    drawCopyright(yOffset);
}

/* 12ba:0003 */
void showStatusMessage(const char* msg)
{
    // TODO implement
    std::cout << msg << std::endl;
}

} // namespace resl
