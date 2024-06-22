#pragma once

#include <cstdint>

namespace resl {

struct RailConnectionBias {
    std::int8_t tileOffsetX;
    std::int8_t tileOffsetY;
    std::uint8_t unknown1;
    std::uint8_t unknown2;
};

/* 1d60:0000 - 48 bytes */
extern const RailConnectionBias g_railConnectionBiases[6][2]; // [road type][direction]

} // namespace resl
