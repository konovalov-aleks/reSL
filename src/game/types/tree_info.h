#pragma once

#include <graphics/color.h>

#include <cstdint>

namespace resl {

enum StaticObjectKind : std::uint8_t {
    House = 2,
    Tree = 3
};

struct StaticObject {
    std::int16_t x;
    std::int16_t y;
    StaticObjectKind kind;
    std::uint8_t type;
    Color color;
    std::uint8_t _yearsElapsed;
};

static_assert(sizeof(StaticObject) == 0x8);

} // namespace resl
