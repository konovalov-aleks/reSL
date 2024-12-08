#include "close_button.h"

#include "button.h"
#include <game/melody.h>
#include <graphics/vga.h>
#include <system/mouse.h>

namespace resl {

static constexpr std::int16_t s_buttonX = 600;
static constexpr std::int16_t s_buttonY = 10;

CloseButton::CloseButton()
{
    m_mouseHandler = Driver::instance().mouse().addHandler(
        [this](MouseEvent& e) {
            if (m_clicked || e.flags() != ME_LEFTRELEASED)
                return;

            const std::int16_t x = e.x();
            std::int16_t y = e.y();
            if (y > 350)
                y -= 350;
            if (x >= s_buttonX && x <= s_buttonX + buttonWidth() &&
                y >= s_buttonY && y <= s_buttonY + buttonHeight()) {

                e.stopPropagation();
                click();
            }
        });
}

void CloseButton::draw(std::int16_t yOffset) noexcept
{
    drawButton(s_buttonX, s_buttonY + yOffset, "X");
}

void CloseButton::click() noexcept
{
    if (!m_clicked) {
        m_clicked = true;
        toggleButtonState(s_buttonX, s_buttonY);
        playEntitySwitchedSound(false);
        vga::waitForNRetraces(8);
    }
}

} // namespace resl
