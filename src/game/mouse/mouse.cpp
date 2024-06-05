#include "mouse.h"

#include "management_mode.h"
#include "mouse_mode.h"
#include "mouse_state.h"
#include <system/mouse.h>
#include <tasks/message_queue.h>
#include <tasks/task.h>

#include <cassert>
#include <cstdint>
#include <iostream>

namespace resl {

using namespace mouse; // FIXME

namespace {

    /* 1d7d:1cce : 32 bytes */
    const MouseMode g_mouseModeRoadConstruction = { /* TODO initialize */ };

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
void handleMouseInput(std::uint16_t mouseEventFlags,
                      std::uint16_t mouseButtonState,
                      std::int16_t dx, std::int16_t dy)
{
    static std::int16_t lastX = 0;
    static std::int16_t lastY = 0;

    // the original game uses a global vairable here but we have a different
    // coroutines implementation, and it makes sense to use local variable instead.
    MsgMouseEvent msg = { MouseAction::None, dx, dy };

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
                if (g_mouseState.mode == &g_modeManagement)
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
                     "   dx = " << e.cursorDX << "\n"
                     "   dy = " << e.cursorDY << std::endl;

        assert(g_mouseState.mode);
        g_mouseState.mode->updatePosFn(e.cursorDX, e.cursorDY);

        co_await sleep(1);
    }
    co_return;
}

} // namespace resl
