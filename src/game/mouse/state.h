#pragma once

#include "mode.h"
#include <graphics/glyph.h>

#include <cstdint>

namespace resl::mouse {

struct State {
    Mode* mode;
    // std::uint16_t padding;
    const Glyph* glyph;
    std::uint16_t videoMemOffset;
};

//-----------------------------------------------------------------------------

/* 262d:6f02 : 12 bytes */
extern State g_state;

//-----------------------------------------------------------------------------

/* 14af:0104 */
void setMode(Mode&);

void toggleMode();

} // namespace resl::mouse
