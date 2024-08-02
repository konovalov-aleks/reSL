#include "construction_mode.h"

#include "mode.h"
#include "state.h"
#include <game/rail_info.h>
#include <game/resources/allowed_cursor_rail_types.h>
#include <game/resources/rail_glyph.h>
#include <graphics/color.h>
#include <graphics/drawing.h>
#include <graphics/glyph.h>
#include <graphics/vga.h>
#include <system/driver/driver.h>
#include <ui/components/status_bar.h>

#include <cassert>
#include <cstdint>

namespace resl::mouse {

namespace {

    /* 14af:0126 */
    void drawCursorRail()
    {
        const RailGlyph* rg = railBackgrounds[g_railCursorState.railType].mainGlyph;
        assert(rg);
        const std::int16_t x =
            (g_railCursorState.tileX - g_railCursorState.tileY) * 88 + rg->dx + 320;
        const std::int16_t y =
            (g_railCursorState.tileX + g_railCursorState.tileY) * 21 + rg->dy - 22;

        g_state.videoMemOffset = y * vga::VIDEO_MEM_ROW_BYTES + x / 8;
        g_state.glyph = &rg->glyph;

        drawGlyphAlignX8(g_state.glyph, x, y, Color::BWBlinking);
    }

    void eraseCursorRail()
    {
        assert(g_state.glyph);
        const Glyph& glyph = *g_state.glyph;

        vga::VideoMemPtr dstPtr = vga::VIDEO_MEM_START_ADDR + g_state.videoMemOffset;
        vga::VideoMemPtr srcPtr = graphics::VIDEO_MEM_SHADOW_BUFFER + g_state.videoMemOffset;

        vga::setVideoModeR0W1();
        const std::uint8_t* glyphData = glyph.data;
        for (std::uint8_t y = 0; y < glyph.height; ++y) {
            for (std::uint8_t x = 0; x < glyph.width; ++x) {
                if (*(glyphData++))
                    Driver::instance().vga().write(dstPtr, Driver::instance().vga().read(srcPtr));
                ++srcPtr;
                ++dstPtr;
            }
            srcPtr += vga::VIDEO_MEM_ROW_BYTES - glyph.width;
            dstPtr += vga::VIDEO_MEM_ROW_BYTES - glyph.width;
        }
        vga::setVideoModeR0W2();
    }

    /* 14af:0264 */
    void updateCursorPos(std::int16_t dx, std::int16_t dy)
    {
        Mode& mode = *g_state.mode;
        mode.x += dx;
        mode.y += dy;
        const std::int8_t dTileX = (mode.y * 88 + mode.x * 21) / 1848;
        const std::int8_t dTileY = (mode.y * 88 - mode.x * 21) / 1848;

        if (!(dTileX || dTileY))
            return;

        mode.x = 0;
        mode.y = 0;

        const std::int8_t dstTileX = g_railCursorState.tileX + dTileX;
        const std::int8_t dstTileY = g_railCursorState.tileY + dTileY;

        /* The initial value of x and y members of g_modeConstruction are:
                x = 320 (1d7d:1cde)
                y = 200 (1d7d:1ce0)
           Thus, at the first call of this function we have dTileX = 13 and dTileY = 5.

           This causes access outside the g_railCursorState array bounds:
                g_railCursorState[13 + 5][5 + 5] (1d3b:0093)

           This looks like a bug, but they were lucky and this value contains 0 =>
           this condition evaluates to false => nothing bad happens.

           I changed the initial values to fix the error, and added an extra check bellow
           for better reliability (not to crash if the handler called with large delta values)
        */
        if (dstTileX < 0 || dstTileX > 10 || dstTileY < 0 || dstTileY > 10)
            return;

        if (g_allowedRailCursorTypes[dstTileX][dstTileY] & (1 << g_railCursorState.railType)) {
            mode.clearFn();
            g_railCursorState.tileX = dstTileX;
            g_railCursorState.tileY = dstTileY;
            mode.drawFn();
        }
    }

} // namespace

/* 1d7d:1ca8 : 6 bytes */
RailInfo g_railCursorState = {
    0, // roadTypeMask
    5, // tileX
    5, // tileY
    0, // railType
    0, // year
};

/* 1d7d:1cce : 32 bytes */
Mode g_modeConstruction = {
    { nullptr,      nullptr      },
    { Color::Black, Color::White },

    -10, // min X
    650, // max X
    0, // min Y
    g_footerYPos, // max Y

    // The original game has different values here (320 / 200).
    // You can find an explanation in the comment to the updateCursorPos function.
    0, // x
    0, // y

    &drawCursorRail,
    &eraseCursorRail,
    &updateCursorPos
};

} // namespace resl::mouse
