#include "loading_screen.h"

#include <graphics/drawing.h>
#include <graphics/vga.h>
#include <system/buffer.h>
#include <system/driver/driver.h>
#include <system/filesystem.h>
#include <system/keyboard.h>
#include <system/mouse.h>

#include <cstdint>

namespace resl {

namespace {

    /* 17a7:0150 */
    void drawLoadingScreenTitle(std::int16_t item)
    {
        vga::waitForLine(70);
        graphics::copyRectangle(132, 10, 8, item * 49 + 351, 49, 46);
        vga::setVideoModeR0W2();
    }

    /* 17a7:0120 */
    void clearLoadingScreenTitle()
    {
        vga::waitForLine(70);
        graphics::copyRectangle(132, 10, 0, 600, 49, 46);
        vga::setVideoModeR0W2();
    }

    class LoadingScreenMouseHandler {
    public:
        LoadingScreenMouseHandler()
            : m_clicked(false)
        {
            m_handler = Driver::instance().mouse().addHandler(
                [this](MouseEvent& e) {
                    if (e.flags()) // any mouse button actions
                        m_clicked = true;
                    e.stopPropagation();
                });
        }

        bool clicked() const noexcept { return m_clicked; }

    private:
        MouseDriver::HandlerHolder m_handler;
        bool m_clicked;
    };

} // namespace

/* 17a7:0005 */
void showLoadingScreen()
{
    g_lastKeyPressed = 0;

    readBinaryFile("poster.7", g_pageBuffer);
    graphics::imageDot7(0, 0, 640, 350, g_pageBuffer);

    readBinaryFile("captions.7", g_pageBuffer);
    graphics::imageDot7(0, 350, 400, 350, g_pageBuffer);

    drawLoadingScreenTitle(0);

    LoadingScreenMouseHandler mouseHandler;

    /** BUGFIX **

       The original game reads the palette from the beginning of "captions.7"
       file and sets it at this point. So, here is the following code:
            setPalette(g_pageBuffer); // 1594:0151

       But the pictures have already been drawn here.

       This is a cause of the bug that the loading screen is drawn with
       incorrect colors at startup time for a moment.

       reSL uses a hardcoded palette => the colors are always correct.
     */

    constexpr std::int16_t nItems = 5;
    constexpr std::int16_t totalAnimationTime = 120;

    for (std::int16_t i = 0; i < nItems + 1; ++i) {
        drawLoadingScreenTitle(i % nItems);
        for (std::int16_t j = 0; j < 220; ++j) {
            if (g_lastKeyPressed || mouseHandler.clicked())
                return;
            vga::waitVerticalRetrace();
        }
        if (i != nItems) {
            clearLoadingScreenTitle();
            vga::waitForNRetraces(totalAnimationTime / nItems);
        }
    }
    g_lastKeyPressed = 0;
}

} // namespace resl
