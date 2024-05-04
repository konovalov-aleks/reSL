#pragma once

#include <cstdint>

namespace resl {

struct DispatcherGlyph {
    std::uint8_t fg[32]; // black
    std::uint8_t bg[32];
};

/* 1d7d:86e8 - 128 bytes */
extern const DispatcherGlyph g_dispatcherGlyphs[2]; // not signalling/signalling

} // namespace resl
