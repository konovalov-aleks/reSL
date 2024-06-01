#include "mouse.h"

#include <graphics/glyph.h>
#include <system/mouse.h>
#include <tasks/message_queue.h>
#include <tasks/task.h>

#include <cstdint>
#include <iostream>

namespace resl {

namespace {

    struct MouseMode {
        // TODO
    };

    struct MouseState {
        MouseMode* mode;
        std::uint8_t unknown1;
        std::uint8_t unknown2;
        Glyph* glyph;
        std::uint16_t videoMemOffset;
    };

    /* 1d7d:1cae : 32 bytes */
    const MouseMode g_mouseMode1 = { /* TODO initialize */ };

    /* 262d:6f02 : 12 bytes */
    const MouseState g_mouseState = { /* TODO initialize */ };

    /* 262d:6ef8 : 1 byte */
    std::uint8_t g_previousMouseButtonState = 0;

    /* The original game uses an own implementation of stack-based coroutines.
       I decided not to restore the sources of this mechanic, because it's
       non-portable and heavily dependent on X86 architecture.

       So, the address below contains a different data structure, but the
       meaning is roughly the same. */

    /* 1d7d:1c96 */
    MessageQueue<MsgMouseEvent> g_mouseMsgQueue;

} // namespace

/* 14af:0761 */
void handleMouseInput(std::uint16_t mouseEventFlags, std::uint16_t mouseButtonState,
                      std::int16_t lastRawHMickeyCount, std::int16_t lastRawVMickeyCount)
{
    // the original game uses a global vairable here but we have a different
    // coroutines implementation, and it makes sense to use local variable instead.
    MsgMouseEvent msg = { MouseAction::None, lastRawHMickeyCount, lastRawVMickeyCount };

    if (g_previousMouseButtonState)
        g_previousMouseButtonState = static_cast<std::uint8_t>(mouseButtonState);
    else {
        // TODO make an enum for mouseButtonState
        if ((mouseEventFlags & (ME_LEFTPRESSED | ME_RIGHTPRESSED)) && mouseButtonState == 3) {
            msg.action = MouseAction::ToggleMouseMode;
            g_previousMouseButtonState = 1;
        } else {
            if (mouseEventFlags & ME_LEFTRELEASED) {
                // left button clicked
                if (g_mouseState.mode == &g_mouseMode1)
                    msg.action = MouseAction::MouseMove;
                else
                    msg.action = MouseAction::ToggleNextRailType;
            } else {
                if (mouseEventFlags & ME_RIGHTRELEASED) {
                    // right button clicked
                    msg.action = MouseAction::BuildRails;
                } else
                    msg.action = MouseAction::CallServer;
            }
        }
    }
    g_mouseMsgQueue.push(msg);
}

/* 14af:0320 */
Task taskMouseEventHandling()
{
    for (;;) {
        MsgMouseEvent e = co_await g_mouseMsgQueue.pop();

        // TODO implement
        std::cout << "mouse event:\n"
                     "   action = " << static_cast<int>(e.action) << "\n"
                     "   x = " << e.cursorX << "\n"
                     "   y = " << e.cursorY << std::endl;

        co_await sleep(1);
    }
    co_return;
}

} // namespace resl
