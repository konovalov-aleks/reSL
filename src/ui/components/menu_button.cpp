#include "menu_button.h"

#include "button.h"
#include <game/melody.h>
#include <graphics/color.h>
#include <graphics/drawing.h>
#include <graphics/vga.h>
#include <system/mouse.h>

namespace resl {

static constexpr std::int16_t s_buttonX = 600;
static constexpr std::int16_t s_buttonY = 13;

MenuButton::MenuButton()
{
    m_mouseHandler = Driver::instance().mouse().addHandler(
        [this](MouseEvent& e) {
            if (!m_enabled || m_clicked || e.flags() != ME_LEFTRELEASED)
                return;

            if (e.x() >= s_buttonX && e.x() <= s_buttonX + buttonWidth() &&
                e.y() >= s_buttonY && e.y() <= s_buttonY + buttonHeight()) {

                e.stopPropagation();
                click();
            }
        });
}

void MenuButton::disable() noexcept
{
    if (!m_enabled) [[unlikely]]
        return;

    m_enabled = false;
    erase();
}

void MenuButton::enable() noexcept
{
    if (m_enabled) [[unlikely]]
        return;

    m_enabled = true;
    draw();
}

void MenuButton::draw(std::int16_t yOffset) noexcept
{
    drawButton(s_buttonX, s_buttonY + yOffset, nullptr);

    // draw a "hamburger" icon
    constexpr std::int16_t iconXOffset = 8;
    constexpr std::int16_t iconYOffset = 5;
    constexpr std::int16_t iconWidth = 11;
    constexpr std::int16_t iconLineHeight = 3;
    constexpr std::int16_t iconLineYSpacing = 1;

    const int16_t x1 = s_buttonX + iconXOffset;
    const int16_t x2 = x1 + iconWidth - 1;
    std::int16_t y = s_buttonY + iconYOffset + yOffset;
    for (int line = 0; line < 3; ++line) {
        for (int i = 0; i < iconLineHeight; ++i) {
            graphics::horizontalLine(x1, x2, y, Color::Black);
            ++y;
        }
        y += iconLineYSpacing;
    }
}

void MenuButton::erase() noexcept
{
    graphics::filledRectangle(
        s_buttonX, s_buttonY,
        buttonWidth() / 8 + 1, buttonHeight(),
        0xFF, Color::Gray);
}

void MenuButton::click() noexcept
{
    m_clicked = true;
    toggleButtonState(s_buttonX, s_buttonY);
    playEntitySwitchedSound(false);
    vga::waitForNRetraces(8);
}

} // namespace resl
