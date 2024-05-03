#pragma once

#include <graphics/glyph.h>

#include <cstdint>

namespace resl {

struct TrainGlyph {
    std::int8_t width;
    std::int8_t height;
    const Glyph* glyph1;
    const Glyph* glyph2;
    const Glyph* glyph3;
};

/* 1cae:0000 : 2100 bytes */
extern const TrainGlyph g_trainGlyphs[15][5][2];

} // namespace resl
