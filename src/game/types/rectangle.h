#pragma once

#include <cstdint>

namespace resl {

struct Rectangle {
    std::int16_t x1;
    std::int16_t y1;
    std::int16_t x2;
    std::int16_t y2;
};

static_assert(sizeof(Rectangle) == 8);

} // namespace resl
