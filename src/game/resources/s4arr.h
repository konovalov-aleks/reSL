#pragma once

#include <cstdint>

namespace resl {

struct s4 {
    std::int8_t tileOffsetX;
    std::int8_t tileOffsetY;
    std::uint8_t unknown1;
    std::uint8_t unknown2;
};

/* 1d60:0000 - 48 bytes */
extern const s4 s4arr[6][2];

} // namespace resl
