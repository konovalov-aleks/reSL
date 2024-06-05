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

/* 1a65:050b */
inline void setEmptyRectangle(Rectangle& r)
{
    r.x1 = 10000;
    r.x2 = -10000;
    r.y1 = 10000;
    r.y2 = -10000;
}

} // namespace resl
