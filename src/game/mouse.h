#pragma once

#include <tasks/task.h>

#include <cstdint>

namespace resl {

enum MouseAction : std::uint8_t {
    None = 0,
    BuildRails = 5,
    ToggleNextRailType = 6,
    ToggleMouseMode = 7,
    CallServer = 8,
    MouseMove = 10
};

struct MsgMouseEvent {
    MouseAction action;
    std::int16_t cursorX;
    std::int16_t cursorY;
};

/* 14af:0761 */
void handleMouseInput(std::uint16_t mouseEventFlags, std::uint16_t mouseButtonState,
                      std::int16_t lastRawHMickeyCount, std::int16_t lastRawVMickeyCount);

/* 14af:0320 */
Task taskMouseEventHandling();

} // namespace resl
