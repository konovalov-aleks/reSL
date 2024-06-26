#pragma once

#include "mouse_mode.h"
#include <graphics/glyph.h>

#include <cstdint>

namespace resl::mouse {

struct MouseState {
    MouseMode* mode;
    std::uint8_t unknown1;
    std::uint8_t unknown2;
    const Glyph* glyph;
    std::uint16_t videoMemOffset;
};

//-----------------------------------------------------------------------------

/* 262d:6f02 : 12 bytes */
extern MouseState g_state;

//-----------------------------------------------------------------------------

/* 14af:0104 */
void setMouseMode(MouseMode&);

} // namespace resl::mouse
