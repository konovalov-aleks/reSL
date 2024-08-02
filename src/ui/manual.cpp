#include "manual.h"

#include "components/button.h"
#include <game/melody.h>
#include <game/mouse/management_mode.h>
#include <game/mouse/mode.h>
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

namespace {

    constexpr std::int16_t closeBtnX = 600;
    constexpr std::int16_t closeBtnY = 360;

    class ManualScreenMouseHandler {
    public:
        ManualScreenMouseHandler()
        {
            m_oldHandler = Driver::instance().setMouseHandler(
                [this](std::uint16_t flags, std::uint16_t /* buttonState */,
                       std::int16_t x, std::int16_t y) {
                    if (m_disabled)
                        return;

                    mouse::g_modeManagement.updatePosFn(x, y);

                    if (!(flags & ME_LEFTRELEASED))
                        return;

                    m_clicked = true;
                    y += 350;

                    if (x >= closeBtnX && x < closeBtnX + buttonWidth() &&
                        y >= closeBtnY && y < closeBtnY + buttonHeight())
                        m_closeBtnPressed = true;
                });
        }

        ~ManualScreenMouseHandler()
        {
            disable();
            Driver::instance().setMouseHandler(m_oldHandler);
        }

        bool clicked() const noexcept { return m_clicked; }
        bool closePressed() const noexcept { return m_closeBtnPressed; }

        void disable()
        {
            if (!m_disabled) {
                m_disabled = true;
                mouse::eraseArrowCursor();
            }
        }

        void enable()
        {
            if (m_disabled)
                mouse::drawArrowCursor();

            m_disabled = false;
            m_clicked = false;
            m_closeBtnPressed = false;
        }

    private:
        MouseHandler m_oldHandler;
        bool m_disabled = true;
        bool m_clicked = false;
        bool m_closeBtnPressed = false;
    };

} // namespace

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

    ManualScreenMouseHandler mouseHandler;

    std::int16_t curPage = 0;
    do {
        graphics::dialogFrame(0, 350, 80, 350, Color::Gray);
        const char* pageData = pages[curPage];
        for (std::int16_t i = 0; i < 25; ++i) {
            drawTextSmall(48, i * 13 + 360, pageData, Color::Black);
            pageData += std::strlen(pageData) + 2;
        }
        drawButton(closeBtnX, closeBtnY, "X");

        mouseHandler.disable();
        graphics::animateScreenShifting();
        graphics::copyScreenBufferTo(0);
        graphics::setVideoFrameOrigin(0, 0);
        mouseHandler.enable();

        g_lastKeyPressed = 0;
        do {
            // The original game has computation-intense loop here
            // (without waitVerticalRetrace call).
            vga::waitVerticalRetrace();
        } while (!g_lastKeyPressed && !mouseHandler.clicked());

        curPage = (curPage + 1) % pageCnt;
    } while (g_lastKeyPressed != g_keyEscape && !mouseHandler.closePressed());

    mouseHandler.disable();
    toggleButtonState(closeBtnX, closeBtnY);
    playEntitySwitchedSound(false);
}

} // namespace resl
