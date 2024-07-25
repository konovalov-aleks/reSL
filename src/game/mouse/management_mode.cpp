#include "management_mode.h"

#include "mouse_mode.h"
#include <graphics/color.h>
#include <graphics/drawing.h>
#include <graphics/glyph.h>
#include <graphics/vga.h>

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <iterator>

namespace resl::mouse {

namespace {

    /* 1d7d:2582 : 32 bytes */
    const std::uint8_t g_cursorGlyph1[] = {
        0x00, 0xc0, 0x00, 0xa0, 0x00, 0x90, 0x00, 0x88, 0x00, 0x84, 0x00, 0x82,
        0x00, 0x81, 0x80, 0x80, 0x40, 0x80, 0x20, 0x80, 0xe0, 0x81, 0x00, 0xb9,
        0x00, 0xc9, 0x80, 0x04, 0x80, 0x04, 0x80, 0x03
    };

    /* 1d7d:25a2 : 32 bytes */
    const std::uint8_t g_cursorGlyph2[] = {
        0x00, 0x00, 0x00, 0x40, 0x00, 0x60, 0x00, 0x70, 0x00, 0x78, 0x00, 0x7c,
        0x00, 0x7e, 0x00, 0x7f, 0x80, 0x7f, 0xc0, 0x7f, 0x00, 0x7c, 0x00, 0x46,
        0x00, 0x06, 0x00, 0x03, 0x00, 0x03, 0x00, 0x00
    };

    /* 14af:000e */
    void updateCursorPos(std::int16_t x, std::int16_t y)
    {
        MouseMode& mode = g_modeManagement;
        mode.clearFn();
        mode.x = std::min(std::max<std::int16_t>(x, mode.minX), mode.maxX);
        mode.y = std::min(std::max<std::int16_t>(y, mode.minY), mode.maxY);
        mode.drawFn();
    }

    constexpr vga::VideoMemPtr g_cursorAreaBackup = vga::VIDEO_MEM_START_ADDR + 0x7861;

} // namespace

/* 14af:0089 */
void drawArrowCursor()
{
    const MouseMode& mode = g_modeManagement;
    graphics::saveVideoMemRegion24x16(mode.x, mode.y, g_cursorAreaBackup);
    vga::setVideoModeR0W2();

    static_assert(std::size(MouseMode {}.glyphs) == std::size(MouseMode {}.colors));
    g_glyphHeight = 16;
    for (std::size_t i = 0; i < std::size(mode.glyphs); ++i)
        drawGlyphW16(mode.glyphs[i], mode.x, mode.y, mode.colors[i]);
}

/* 14af:00e6 */
void eraseArrowCursor()
{
    const MouseMode& mode = g_modeManagement;
    graphics::restoreVideoMemRegion24x16(mode.x, mode.y, g_cursorAreaBackup);
    vga::setVideoModeR0W2();
}

//-----------------------------------------------------------------------------

/* 1d7d:1cae : 32 bytes */
MouseMode g_modeManagement = {
    { g_cursorGlyph1, g_cursorGlyph2 },
    { Color::Black,   Color::White   },
    0, // min X
    628, // max X
    47, // min Y
    318, // max Y
    320, // x
    200, // y
    &drawArrowCursor,
    &eraseArrowCursor,
    &updateCursorPos
};

} // namespace resl::mouse
