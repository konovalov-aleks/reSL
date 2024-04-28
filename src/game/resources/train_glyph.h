#pragma once

#include <graphics/glyph.h>

#include <cstdint>

namespace resl {

struct TrainGlyph {
    std::int8_t dx;
    std::int8_t dy;
    const Glyph* glyph1;
    const Glyph* glyph2;
    const Glyph* glyph3;
};

/* 1cae:0000 : 2100 bytes */
extern const TrainGlyph trainGlyphs[15][10];

} // namespace resl
