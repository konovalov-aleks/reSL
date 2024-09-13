#pragma once

#include <cstdint>
#include <functional>

namespace resl {

/*  The original game uses the MOUSE/MOUSE.H TurboC library.

    They register a handler (12b6:0001) by calling SETMOUSEHANDLE (15ab:0008).
    The mouse handler just calls handleMouseInput function (14af:0761) and
    calls the 0x33 interruption to reset the movement delta.

    I use SDL instead of this non-portable DOS API.

    This file is a kind of emulation of TurboC MOUSE.H library.
*/

// constants from MOUSE.H
enum MouseEventType : std::uint16_t {
    ME_MOVED = 1,
    ME_LEFTPRESSED = 2,
    ME_LEFTRELEASED = 4,
    ME_RIGHTPRESSED = 8,
    ME_RIGHTRELEASED = 16,
    ME_CENTERPRESSED = 32,
    ME_CENTERRELEASED = 64
};

/* From the TurboC MOUSE/MOUSE.H header:
    271         BX = button state (bit 0 set if left button down, bit 1 set if right
    272                            button down and bit 2 set if center button down)
 */
enum MouseButton {
    MB_NONE = 0,
    MB_LEFT = 1,
    MB_RIGHT = 2,
    MB_MIDDLE = 4
};

class MouseEvent {
public:
    MouseEvent(
        std::uint16_t flags, std::uint16_t btnState,
        std::int16_t x, std::int16_t y)
        : m_flags(flags)
        , m_buttonState(btnState)
        , m_x(x)
        , m_y(y)
    {
    }

    std::uint16_t flags() const noexcept { return m_flags; }
    std::uint16_t buttonState() const noexcept { return m_buttonState; }

    std::int16_t x() const noexcept { return m_x; }
    std::int16_t y() const noexcept { return m_y; }

    void stopPropagation() noexcept { m_propagation = false; }
    bool propagation() const noexcept { return m_propagation; }

private:
    std::uint16_t m_flags;
    std::uint16_t m_buttonState;
    std::int16_t m_x;
    std::int16_t m_y;

    bool m_propagation = true;
};

using MouseHandler = std::function<void(MouseEvent&)>;

} // namespace resl
