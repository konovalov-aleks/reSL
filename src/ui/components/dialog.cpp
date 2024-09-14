#include "dialog.h"

#include "button.h"
#include <game/constants.h>
#include <game/melody.h>
#include <graphics/color.h>
#include <graphics/drawing.h>
#include <graphics/text.h>
#include <graphics/vga.h>
#include <system/driver/driver.h>
#include <system/keyboard.h>
#include <system/mouse.h>
#include <types/rectangle.h>

#include <cassert>
#include <cstdint>
#include <iterator>
#include <optional>

namespace resl {

namespace {

    /* 262d:6f2a : 8 bytes */
    Rectangle g_dialogArea;

    /* 1d7d:1f62 : 74 bytes */
    const std::uint8_t g_asciiToKeycodeTable[74] = {
        0x1E, 0x30, 0x2E, 0x20, 0x12, 0x21, 0x22, 0x23, 0x17, 0x24, 0x25, 0x26,
        0x32, 0x31, 0x18, 0x19, 0x10, 0x13, 0x1F, 0x14, 0x16, 0x2F, 0x11, 0x2D,
        0x15, 0x2C, 0x46, 0x20, 0x7D, 0x1D, 0x4A, 0x20, 0x7D, 0x1D, 0x4E, 0x20,
        0x7D, 0x1D, 0x52, 0x20, 0x7D, 0x1D, 0x56, 0x20, 0x7D, 0x1D, 0x5A, 0x20,
        0x7D, 0x1D, 0x5E, 0x20, 0x7D, 0x1D, 0x62, 0x20, 0x7D, 0x1D, 0x66, 0x20,
        0x7D, 0x1D, 0x6A, 0x20, 0x7D, 0x1D, 0x6E, 0x20, 0x7D, 0x1D, 0x72, 0x20,
        0x7D, 0x1D
    };

    class DialogMouseHandler {
    public:
        DialogMouseHandler(DialogType dt)
            : m_type(dt)
        {
            Driver::instance().mouse().setCursorVisibility(true);
            m_oldHandler = Driver::instance().mouse().setHandler(
                [this](std::uint16_t flags, std::uint16_t buttonState,
                       std::int16_t x, std::int16_t y) {
                    handle(flags, buttonState, x, y);
                });
        }

        ~DialogMouseHandler()
        {
            Driver::instance().mouse().setHandler(m_oldHandler);
        }

        bool newClicks()
        {
            bool res = m_clicked;
            m_clicked = false;
            return res;
        }

        std::optional<std::int16_t> selectedItem() const { return m_selectedItem; }

        DialogMouseHandler(const DialogMouseHandler&) = delete;
        DialogMouseHandler& operator=(const DialogMouseHandler&) = delete;

    private:
        void handle(std::uint16_t flags, std::uint16_t /* buttonState */,
                    std::int16_t x, std::int16_t y)
        {
            if (!(flags & ME_LEFTRELEASED))
                return;

            m_clicked = true;
            const Dialog& dlg = g_dialogs[static_cast<int>(m_type)];
            if (x < dlg.x || x > dlg.x + dlg.itemWidth)
                return;

            const int itemHeight = buttonHeight();
            for (std::int16_t i = 0; dlg.itemNames[i]; ++i) {
                std::int16_t dlgY = dlg.itemY[i];
                if (dlgY > 350)
                    dlgY -= 350; // in the shadow buffer
                if (y >= dlgY && y <= dlgY + itemHeight) {
                    m_selectedItem = i;
                    return;
                }
            }
        }

        MouseHandler m_oldHandler;
        DialogType m_type;
        std::optional<std::int16_t> m_selectedItem;
        bool m_clicked = false;
    };

} // namespace

/* 1d65:0000 : 240 bytes */
Dialog g_dialogs[5] = {
    { "MAIN  MENU",
     { "Manual",
        "Demo",
        "Go!",
        "Archive",
        "Records",
        "Bye" } },
    { "ARCHIVE",
     { "View",
        "Next",
        "Go!",
        "Delete",
        "Bye" } },
    { "Sure?",
     { "Yes",
        "No" }  },
    { "PAUSE",
     { "Go!",
        "",
        "Bye" } },
    { "?",
     {}         }
};

//-----------------------------------------------------------------------------

/* 15e8:0003 */
Rectangle& drawDialog(DialogType type, std::int16_t yOffset)
{
    Dialog& dlg = g_dialogs[static_cast<int>(type)];

    g_textSpacing = 3;

    std::int16_t maxWidth = measureText(dlg.title);

    char firstSymbols[7][2];
    std::int16_t itemsCount = 0;
    for (; itemsCount < 7 && dlg.itemNames[itemsCount]; ++itemsCount) {
        std::int16_t itemWidth = measureText(dlg.itemNames[itemsCount]);
        if (itemWidth > maxWidth)
            maxWidth = itemWidth;
        firstSymbols[itemsCount][0] = dlg.itemNames[itemsCount][0];
        firstSymbols[itemsCount][1] = '\0';
    }
    dlg.itemWidth = maxWidth;

    const std::int16_t widthBytes = (maxWidth + 56) / 8;
    const std::int16_t height = (itemsCount + 1) * 30 + 18;

    // the same in pixels is:
    //  x = (640 - width) / 2
    const std::int16_t x = (80 - widthBytes) * 4;
    const std::int16_t y0 = yOffset + (GAME_FIELD_HEIGHT - height) / 2;
    const std::int16_t y = y0 + 47;

    const Color bgColor = itemsCount ? Color::Gray : Color::Red;
    graphics::dialogFrame(x, y, widthBytes, height, bgColor);
    drawText(x + 25, y0 + 65, dlg.title, Color::Black);
    for (std::int16_t i = 0; i < itemsCount; ++i) {
        dlg.x = x + 25;
        const std::int16_t itemBaseLineY = y + (i + 1) * 30;
        const std::int16_t itemY = itemBaseLineY + 18;
        dlg.itemY[i] = itemY;
        drawButton(dlg.x, itemY, firstSymbols[i]);
        drawText(dlg.x + 28, itemBaseLineY + 23, &dlg.itemNames[i][1], Color::Black);
    }

    g_dialogArea.x1 = x;
    g_dialogArea.x2 = x + widthBytes * 8;
    g_dialogArea.y1 = y;
    if (y > 350)
        g_dialogArea.y1 -= 350;
    g_dialogArea.y2 = g_dialogArea.y1 + height + 1;
    return g_dialogArea;
}

/* 15e8:03b7 */
std::int16_t handleDialog(DialogType type, std::optional<std::int16_t> defaultChoice)
{
    const Dialog& dlg = g_dialogs[static_cast<int>(type)];
    std::int16_t timeout = 700;
    std::int16_t itemsCount = 0;
    const std::int16_t maxItemsCount =
        static_cast<std::int16_t>(std::size(dlg.itemNames));
    while (itemsCount < maxItemsCount && dlg.itemNames[itemsCount])
        ++itemsCount;

    std::optional<std::int16_t> selectedItem;
    {
        DialogMouseHandler mouseHandler(type);
        for (;;) {
            while (!g_lastKeyPressed && !mouseHandler.newClicks()) {
                if (timeout-- == 0)
                    return -1;
                vga::waitVerticalRetrace();
            }

            if (g_lastKeyPressed) {
                for (std::int16_t i = 0; !selectedItem && i < itemsCount; ++i) {
                    if (dlg.itemNames[i][0] >= 'A' &&
                        g_asciiToKeycodeTable[dlg.itemNames[i][0] - 'A'] == g_lastKeyPressed) {

                        selectedItem = i;
                    }
                }
            } else
                selectedItem = mouseHandler.selectedItem();

            g_lastKeyPressed = 0;
            if (selectedItem || defaultChoice)
                break;

            playErrorMelody();
        }
    }
    if (selectedItem) {
        toggleButtonState(dlg.x, dlg.itemY[*selectedItem]);
        playEntitySwitchedSound(false);
        vga::waitForNRetraces(8);
        return *selectedItem;
    }
    assert(defaultChoice);
    return *defaultChoice;
}

/* 15e8:0947 */
void alert(const char* message)
{
    g_dialogs[static_cast<int>(DialogType::Alert)].title = message;
    vga::waitForLine(200);
    drawDialog(DialogType::Alert, 0);
    handleDialog(DialogType::Alert, 0);
}

} // namespace resl
