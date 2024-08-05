#include "management_mode.h"

#include "mode.h"
#include <graphics/color.h>

#include <algorithm>
#include <cstdint>

namespace resl::mouse {

namespace {

    void updateCursorPos(std::int16_t x, std::int16_t y)
    {
        Mode& mode = g_modeManagement;
        mode.x = std::min(std::max<std::int16_t>(x, mode.minX), mode.maxX);
        mode.y = std::min(std::max<std::int16_t>(y, mode.minY), mode.maxY);
    }

} // namespace

/* 14af:0089 */
void drawArrowCursor()
{
}

static void eraseArrowCursor()
{
}

//-----------------------------------------------------------------------------

/* 1d7d:1cae : 32 bytes */
Mode g_modeManagement = {
    { nullptr,      nullptr      },
    { Color::Black, Color::White },
    0, // min X
    628, // max X
    0, // min Y
    318, // max Y
    320, // x
    200, // y
    &drawArrowCursor,
    &eraseArrowCursor,
    &updateCursorPos
};

} // namespace resl::mouse
