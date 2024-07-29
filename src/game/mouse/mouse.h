#pragma once

#include <tasks/message_queue.h>
#include <tasks/task.h>

#include <cstdint>

namespace resl {

enum MouseAction : std::uint8_t {
    None = 0,
    ToggleSound = 3,
    BuildRails = 5,
    ToggleNextRailType = 6,
    ToggleMouseMode = 7,
    CallServer = 8,
    MouseClick = 10
};

struct MsgMouseEvent {
    MouseAction action;
    std::int16_t cursorDX;
    std::int16_t cursorDY;
};

//-----------------------------------------------------------------------------

/* The original game uses an own implementation of stack-based coroutines.
   I decided not to restore the sources of this mechanic, because it's
   non-portable and heavily dependent on X86 architecture.

   So, the address below contains a different data structure, but the
   meaning is roughly the same. */

/* 1d7d:1c96 */
extern MessageQueue<MsgMouseEvent> g_mouseMsgQueue;

//-----------------------------------------------------------------------------

/* 14af:0761 */
void handleMouseInput(std::uint16_t mouseEventFlags,
                      std::uint16_t mouseButtonState,
                      std::int16_t dx, std::int16_t dy);

/* 14af:0320 */
Task taskMouseEventHandling();

} // namespace resl
