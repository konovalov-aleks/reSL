#pragma once

#include <cstdint>

namespace resl {

struct Path {
    std::uint16_t size;
    std::uint16_t data[940];
};

/* 1d7d:02d4 - 6594 bytes */
extern const Path g_movementPaths[7];

} // namespace resl
