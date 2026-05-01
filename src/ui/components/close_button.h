#pragma once

#include <system/driver/driver.h>

#include <cstdint>

namespace resl {

// The button with "X" icon to close the current screen
class CloseButton {
public:
    CloseButton();

    void click() noexcept;

    bool clicked() const noexcept { return m_clicked; }

    static void draw(std::int16_t yOffset) noexcept;

private:
    MouseDriver::HandlerHolder m_mouseHandler;
    bool m_clicked = false;
};

} // namespace resl
