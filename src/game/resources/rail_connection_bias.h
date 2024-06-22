#pragma once

#include <cstdint>

namespace resl {

// The information about which tiles the rail connects to
struct RailConnectionBias {
    std::int8_t tileOffsetX;
    std::int8_t tileOffsetY;
    std::uint8_t unknown1;
    std::uint8_t _padding;
};

/* 1d60:0000 - 48 bytes */
extern const RailConnectionBias g_railConnectionBiases[6][2]; // [roadType][side]

} // namespace resl
