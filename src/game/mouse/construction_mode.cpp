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
    void updateCursorPos(std::int16_t x, std::int16_t y)
    {
        Mode& mode = *g_state.mode;
        /*
          The projection is:

            x = (tileX - tileY) * 88 + 320
            y = (tileX + tileY) * 21 - 22

            => tileY = (y + 22 - 21 * tileX) / 21
                     = y / 21 + 22/21 - tileX

            => tileX = x + 88 * (y / 21 + 22/21 - tileX) - 320
                     = (x * 21 + 88 * y) / 3696 + 1936 / 3696 - 320 / 176
                     = (x * 21 + 88 * y) / 3696 - 299 / 231

        */
        const double tx = (x * 21 + 88 * y) / 3696.0 - 299.0 / 231.0;
        const std::int8_t tileX = static_cast<std::int8_t>(tx);
        const std::int8_t tileY = static_cast<std::int8_t>(
            y / 21.0 - tx + 22.0 / 21.0);

        if (tileX == mode.x && tileY == mode.y)
            return;

        if (tileX < 0 || tileX > 10 || tileY < 0 || tileY > 10)
            return;

        if (g_allowedRailCursorTypes[tileX][tileY] & (1 << g_railCursorState.railType)) {
            mode.clearFn();
            g_railCursorState.tileX = tileX;
            g_railCursorState.tileY = tileY;
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
