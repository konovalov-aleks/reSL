#include "loading_screen.h"

#include "keyboard.h"
#include <graphics/drawing.h>
#include <graphics/vga.h>
#include <system/buffer.h>
#include <system/filesystem.h>

#include <cstdint>

namespace resl {

/* 17a7:0150 */
static void drawLoadingScreenTitle(std::int16_t item)
{
    vga::waitForLine(70);
    graphics::copyRectangle(132, 10, 8, item * 49 + 351, 49, 46);
    vga::setVideoModeR0W2();
}

/* 17a7:0120 */
static void clearLoadingScreenTitle()
{
    vga::waitForLine(70);
    graphics::copyRectangle(132, 10, 0, 600, 49, 46);
    vga::setVideoModeR0W2();
}

/* 17a7:0005 */
void showLoadingScreen()
{
    g_lastKeyPressed = 0;

    readFromFile("poster.7", g_pageBuffer);
    graphics::imageDot7(0, 0, 640, 350, g_pageBuffer);

    readFromFile("captions.7", g_pageBuffer);
    graphics::imageDot7(0, 350, 400, 350, g_pageBuffer);

    drawLoadingScreenTitle(0);

    /** BUGFIX **

       The original game reads the palette from the beginning of "captions.7"
       file and sets it at this point. So, here is the following code:
            setPalette(g_pageBuffer); // 1594:0151

       But the pictures have already been drawn here.

       This is a cause of the bug that the loading screen is drawn with
       incorrect colors at startup time for a moment.

       reSL uses a hardcoded palette => the colors are always correct.
     */

    for (std::int16_t i = 0; i < 5; ++i) {
        drawLoadingScreenTitle(i % 4);
        for (std::int16_t j = 0; j < 220; ++j) {
            if (g_lastKeyPressed)
                return;
            vga::waitVerticalRetrace();
        }
        if (i != 4) {
            clearLoadingScreenTitle();
            vga::waitForNRetraces(30);
        }
    }
    g_lastKeyPressed = 0;
}

} // namespace resl
