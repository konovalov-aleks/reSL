#include "dialog.h"

#include "keyboard.h"
#include "melody.h"
#include "types/rectangle.h"
#include <graphics/color.h>
#include <graphics/drawing.h>
#include <graphics/glyph.h>
#include <graphics/text.h>
#include <graphics/vga.h>

#include <cassert>
#include <cstdint>

namespace resl {

namespace {

    /* 262d:6f2a : 8 bytes */
    Rectangle g_dialogArea;

    /* 1d7d:884a : 65 bytes */
    const GlyphData<3, 21> g_menuItemGlyph1 = {
        0x7F, 0xFF, 0xFE, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
        0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
        0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
        0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
        0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFD, 0xE0, 0x00, 0x01,
        0x7F, 0xFF, 0xFE
    };

    /* 1d7d:87f4 : 86 bytes */
    const GlyphData<4, 21> g_menuItemGlyph2 = {
        0x7F, 0xFF, 0xFF, 0x80, 0x80, 0x00, 0x00, 0xC0, 0x8F, 0xFF, 0xFF, 0xC0,
        0x90, 0x00, 0x01, 0xC0, 0x90, 0x00, 0x01, 0xC0, 0x90, 0x00, 0x01, 0xC0,
        0x90, 0x00, 0x01, 0xC0, 0x90, 0x00, 0x01, 0xC0, 0x90, 0x00, 0x01, 0xC0,
        0x90, 0x00, 0x01, 0xC0, 0x90, 0x00, 0x01, 0xC0, 0x90, 0x00, 0x01, 0xC0,
        0x90, 0x00, 0x01, 0xC0, 0x90, 0x00, 0x01, 0xC0, 0x90, 0x00, 0x01, 0xC0,
        0x90, 0x00, 0x01, 0xC0, 0x90, 0x00, 0x01, 0xC0, 0x90, 0x00, 0x01, 0xC0,
        0x9F, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xFF, 0xC0, 0x7F, 0xFF, 0xFF, 0x80
    };

    /* 1d7d:879e : 86 bytes */
    const GlyphData<4, 21> g_menuItemGlyph3 = {
        0x7F, 0xFF, 0xFF, 0x80, 0x80, 0x00, 0x00, 0x40, 0x8F, 0xFF, 0xFE, 0x40,
        0x90, 0x00, 0x01, 0x40, 0x90, 0x00, 0x01, 0x40, 0x90, 0x00, 0x01, 0x40,
        0x90, 0x00, 0x01, 0x40, 0x90, 0x00, 0x01, 0x40, 0x90, 0x00, 0x01, 0x40,
        0x90, 0x00, 0x01, 0x40, 0x90, 0x00, 0x01, 0x40, 0x90, 0x00, 0x01, 0x40,
        0x90, 0x00, 0x01, 0x40, 0x90, 0x00, 0x01, 0x40, 0x90, 0x00, 0x01, 0x40,
        0x90, 0x00, 0x01, 0x40, 0x90, 0x00, 0x01, 0x40, 0x90, 0x00, 0x01, 0x40,
        0x8F, 0xFF, 0xFE, 0x40, 0x80, 0x00, 0x00, 0x40, 0x7F, 0xFF, 0xFF, 0x80
    };

    /* 1d74:0000 : 47 bytes */
    const GlyphData<3, 15> g_firstMenuItemTextArea = {
        0XFF, 0XFF, 0XE0, 0XFF, 0XFF, 0XE0, 0XFF, 0XFF, 0XE0, 0XFF, 0XFF, 0XE0,
        0XFF, 0XFF, 0XE0, 0XFF, 0XFF, 0XE0, 0XFF, 0XFF, 0XE0, 0XFF, 0XFF, 0XE0,
        0XFF, 0XFF, 0XE0, 0XFF, 0XFF, 0XE0, 0XFF, 0XFF, 0XE0, 0XFF, 0XFF, 0XE0,
        0XFF, 0XFF, 0XE0, 0XFF, 0XFF, 0XE0, 0XFF, 0XFF, 0XE0
    };

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

/* 15e8:047b */
void highlightFirstDlgItemSymbol(std::int16_t x, std::int16_t y)
{
    if (y > 350)
        y -= 350;

    vga::setDataRotation(0x18); // rotation = 0, mode = XOR
    drawGlyph(g_firstMenuItemTextArea, x + 4, y + 3, Color::Yellow);
    vga::setDataRotation(0);
}

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

    const std::int16_t widthBytes = (maxWidth + 56) / 8;
    const std::int16_t height = (itemsCount + 1) * 30 + 18;

    // the same in pixels is:
    //  x = (640 - width) / 2
    const std::int16_t x = (80 - widthBytes) * 4;
    const std::int16_t y0 = yOffset + (287 - height) / 2;
    const std::int16_t y = y0 + 47;

    const Color bgColor = itemsCount ? Color::Gray : Color::Red;
    drawing::dialogFrame(x, y, widthBytes, height, bgColor);
    drawText(x + 25, y0 + 65, dlg.title, Color::Black);
    for (std::int16_t i = 0; i < itemsCount; ++i) {
        dlg.x = x + 25;
        const std::int16_t itemBaseLineY = y + (i + 1) * 30;
        const std::int16_t itemY = itemBaseLineY + 18;
        dlg.itemY[i] = itemY;
        drawGlyph(g_menuItemGlyph1, dlg.x, itemY, Color::White);
        drawGlyph(g_menuItemGlyph2, dlg.x, itemY, Color::Gray);
        drawGlyph(g_menuItemGlyph3, dlg.x, itemY, Color::Black);
        drawText(dlg.x + 7, itemBaseLineY + 23, firstSymbols[i], Color::Black);
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
std::int16_t handleDialog(DialogType type)
{
    const Dialog& dlg = g_dialogs[static_cast<int>(type)];
    std::int16_t timeout = 700;
    std::int16_t itemsCount = 0;
    while (itemsCount < 7 && dlg.itemNames[itemsCount])
        ++itemsCount;
    while (!g_lastKeyPressed) {
        if (timeout-- == 0)
            return -1;
        vga::waitVerticalRetrace();
    }

    std::int16_t i = 0;
    for (; i < itemsCount; ++i) {
        assert(dlg.itemNames[i][0] >= 'A');
        if (g_asciiToKeycodeTable[dlg.itemNames[i][0] - 'A'] == g_lastKeyPressed) {
            highlightFirstDlgItemSymbol(dlg.x, dlg.itemY[i]);
            playEntitySwitchedSound(false);
            vga::waitForNRetraces(8);
            break;
        }
    }
    g_lastKeyPressed = 0;
    return i;
}

} // namespace resl
