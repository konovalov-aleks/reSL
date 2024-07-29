#pragma once

#include <graphics/color.h>

#include <cstdint>

namespace resl::mouse {

struct Mode {
    using DrawCursorFn =
        void (*)();
    using UpdateCursorPosFn =
        void (*)(std::int16_t /* x */, std::int16_t /* y */);

    const std::uint8_t* const glyphs[2];
    const Color colors[2];
    const std::int16_t minX;
    const std::int16_t maxX;
    const std::int16_t minY;
    const std::int16_t maxY;

    std::int16_t x;
    std::int16_t y;

    const DrawCursorFn drawFn;
    const DrawCursorFn clearFn;
    const UpdateCursorPosFn updatePosFn;
};

} // namespace resl::mouse
