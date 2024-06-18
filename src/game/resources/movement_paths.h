#pragma once

#include <cstdint>

namespace resl {

struct PathStep {
    std::int8_t dx;
    std::int8_t dy;
    std::uint8_t angle;
};

struct Path {
    std::uint16_t size;
    PathStep data[235];
};

/* 1d7d:02d4 - 6594 bytes */
extern const Path g_movementPaths[7]; // [chunk type]

} // namespace resl
