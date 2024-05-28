#pragma once

#include "color.h"

#include <cstdint>

namespace resl {

/* 1d7d:267a : 1 byte */
extern std::uint8_t g_glyphHeight;

struct Glyph {
    std::uint8_t width;
    std::uint8_t height;
    std::uint8_t data[1];
};

/* 1b06:062b */
void drawGlyphAlignX8(const Glyph*, std::int16_t x, std::int16_t y, Color);

/* 1b06:067e */
void drawGlyph(const Glyph*, std::int16_t x, std::int16_t y, Color);

/* 1b06:05cb */
void drawGlyphW8(const std::uint8_t* glyph, std::int16_t x, std::int16_t y, Color);

/* 1b06:0548 */
void drawGlyphW16(const std::uint8_t* glyph, std::int16_t x, std::int16_t y, Color);

template <std::uint8_t W, std::uint8_t H>
struct GlyphData {
    template <typename... Values>
    constexpr GlyphData(Values... values)
        : width(W)
        , height(H)
        , data { static_cast<std::uint8_t>(values)... }
    {
        static_assert(sizeof...(Values) == W * H);
    }

    std::uint8_t width;
    std::uint8_t height;
    std::uint8_t data[W * H];
};

} // namespace resl
