#include "manual.h"

#include "components/close_button.h"
#include <graphics/animation.h>
#include <graphics/color.h>
#include <graphics/drawing.h>
#include <graphics/text.h>
#include <graphics/vga.h>
#include <system/buffer.h>
#include <system/driver/driver.h>
#include <system/filesystem.h>
#include <system/keyboard.h>
#include <system/mouse.h>

#include <cassert>
#include <cstdint>
#include <cstring>

namespace resl {

/* 16a6:04a8 */
void showManual()
{
    constexpr int maxPages = 20;

    std::size_t nBytes = readTextFile("RULES.TXT");

    // split the text with pages 25 lines each
    std::int16_t pageCnt = 0;
    char* buf = reinterpret_cast<char*>(g_pageBuffer);
    const char* const bufEnd = buf + nBytes;
    const char* pages[maxPages];
    while (buf < bufEnd) {
        assert(pageCnt < maxPages);
        pages[pageCnt++] = buf;
        for (std::int16_t i = 0; i < 25; ++i) {
            if (buf < bufEnd) {
                for (; *buf != '\r'; ++buf) { }
                assert(buf[0] == '\r');
                assert(buf[1] == '\n');
            }
            *buf = '\0';
            buf += 2;
        }
    }

    bool needSwitchPage = false;
    MouseDriver::HandlerHolder screenClickHandler =
        Driver::instance().mouse().addHandler(
            [&needSwitchPage](MouseEvent& e) {
                if (e.flags() == ME_LEFTRELEASED) {
                    needSwitchPage = true;
                    e.stopPropagation();
                }
            });

    CloseButton closeButton;

    std::int16_t curPage = 0;
    do {
        graphics::dialogFrame(0, 350, 80, 350, Color::Gray);
        const char* pageData = pages[curPage];
        for (std::int16_t i = 0; i < 25; ++i) {
            drawTextSmall(48, i * 13 + 360, pageData, Color::Black);
            pageData += std::strlen(pageData) + 2;
        }
        closeButton.draw(350);

        graphics::animateScreenShifting();
        graphics::copyScreenBufferTo(0);
        graphics::setVideoFrameOrigin(0, 0);

        g_lastKeyPressed = 0;
        do {
            // The original game has computation-intense loop here
            // (without waitVerticalRetrace call).
            vga::waitVerticalRetrace();
        } while (!g_lastKeyPressed && !closeButton.clicked() && !needSwitchPage);

        curPage = (curPage + 1) % pageCnt;
        needSwitchPage = false;
    } while (g_lastKeyPressed != g_keyEscape && !closeButton.clicked());

    closeButton.click();
}

} // namespace resl
