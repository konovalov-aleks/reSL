#pragma once

#include <system/driver/driver.h>

#include <cstdint>

namespace resl {

// The button with "hamburger" icon in the right top corner of the screen.
class MenuButton {
public:
    MenuButton();

    void disable() noexcept;
    void enable() noexcept;

    void click() noexcept;

    bool clicked() const noexcept { return m_clicked; }
    void reset() noexcept { m_clicked = false; }

    bool handleMouseClick(std::int16_t x, std::int16_t y) noexcept;

    static void draw(std::int16_t yOffset = 0) noexcept;

private:
    void erase() noexcept;

    MouseDriver::HandlerHolder m_mouseHandler;

    bool m_enabled = false;
    bool m_clicked = false;
};

} // namespace resl
