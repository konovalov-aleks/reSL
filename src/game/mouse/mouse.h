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
    MouseClick = 10
};

struct MsgMouseEvent {
    MouseAction action;
    std::int16_t cursorDX;
    std::int16_t cursorDY;
};

/* 14af:0761 */
void handleMouseInput(std::uint16_t mouseEventFlags,
                      std::uint16_t mouseButtonState,
                      std::int16_t dx, std::int16_t dy);

/* 14af:0320 */
Task taskMouseEventHandling();

} // namespace resl
