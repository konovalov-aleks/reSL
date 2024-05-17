#include "text.h"

#include "glyph.h"

namespace resl {

struct GlyphInfo {
    std::uint8_t verticalOffset;
    std::uint8_t height;
    std::uint8_t width;
    const char* glyph;
};

static const struct GlyphInfo g_glyphs[147] = {
    // ' '
    { 0,  1,  12, "\x00\x00"                                                            },
    // '!'
    { 0,  12, 4,  "\x60\xf0\xf0\xf0\xf0\xf0\x60\x60\x60\x00\xf0\xf0"                    },
    // '"'
    { 0,  5,  9,  "\x80\xe3\x80\xe3\x80\xe3\x00\x63\x00\x63"                            },
    // '#'
    { 1,  10, 12,
     "\xc0\x39\xc0\x39\xf0\xff\xf0\xff\xc0\x39\xc0\x39\xf0\xff\xf0\xff\xc0\x39\xc0\x39" },
    // '$'
    { 0,  12, 13,
     "\x00\x07\x00\x07\xf0\x7f\xf8\xff\x00\xe7\xf0\xff\xf8\x7f\x38\x07\xf8\xff\xf0\x7f\x00\x07\x00"
      "\x07"                                                                            },
    // '%'
    { 0,  12, 15,
     "\x1e\x7c\x3c\xc6\x78\xc6\xf0\x7c\xe0\x01\xc0\x03\x80\x07\x00\x0f\x7c\x1e\xc6\x3c\xc6\x78\x7c"
      "\xf0"                                                                            },
    // '&'
    { 0,  12, 14,
     "\x80\x1f\xc0\x3f\xc0\x39\xc0\x39\x80\x3b\x00\x1f\x3c\x3e\x78\x77\xf0\xe3\xe0\xe1\xf8\xff\xbc"
      "\x3f"                                                                            },
    // '''
    { 0,  4,  3,  "\xe0\xe0\x60\xc0"                                                    },
    // '('
    { 0,  12, 7,  "\x1e\x78\xf0\xe0\xe0\xe0\xe0\xe0\xe0\xf0\x78\x1e"                    },
    // ')'
    { 0,  12, 7,  "\xf0\x3c\x1e\x0e\x0e\x0e\x0e\x0e\x0e\x1e\x3c\xf0"                    },
    // '*'
    { 3,  8,  14, "\x78\x78\xf0\x3c\xe0\x1f\xfc\xff\xfc\xff\xe0\x1f\xf0\x3c\x78\x78"    },
    // '+'
    { 3,  8,  11, "\x00\x0e\x00\x0e\x00\x0e\xe0\xff\xe0\xff\x00\x0e\x00\x0e\x00\x0e"    },
    // ','
    { 10, 4,  3,  "\xe0\xe0\x60\xc0"                                                    },
    // '-'
    { 6,  2,  11, "\xe0\xff\xe0\xff"                                                    },
    // '.'
    { 10, 2,  3,  "\xe0\xe0"                                                            },
    // '/'
    { 0,  12, 15,
     "\x1e\x00\x3c\x00\x78\x00\xf0\x00\xe0\x01\xc0\x03\x80\x07\x00\x0f\x00\x1e\x00\x3c\x00\x78\x00"
      "\xf0"                                                                            },
    // '0'
    { 0,  12, 12,
     "\xc0\x3f\xe0\x7f\xf0\xf0\x70\xe0\x70\xe0\x70\xe0\x70\xe0\x70\xe0\x70\xe0\xf0\xf0\xe0\x7f\xc0"
      "\x3f"                                                                            },
    // '1'
    { 0,  12, 11,
     "\x00\x0e\x00\x1e\x00\x7e\x00\x0e\x00\x0e\x00\x0e\x00\x0e\x00\x0e\x00\x0e\x00\x0e\xe0\xff\xe0"
      "\xff"                                                                            },
    // '2'
    { 0,  12, 12,
     "\xc0\x3f\xe0\x7f\xf0\xf0\x70\x00\xf0\x00\xe0\x01\x80\x07\x00\x1e\x00\x78\x00\xf0\xf0\xff\xf0"
      "\xff"                                                                            },
    // '3'
    { 0,  12, 12,
     "\xc0\x3f\xe0\x7f\xf0\xf0\x70\x00\x70\x00\xe0\x0f\xe0\x0f\x70\x00\x70\x00\xf0\xf0\xe0\x7f\xc0"
      "\x3f"                                                                            },
    // '4'
    { 0,  12, 14,
     "\xe0\x01\xe0\x03\xe0\x07\xe0\x0f\xe0\x1e\xe0\x3c\xe0\x78\xe0\xf0\xfc\xff\xfc\xff\xe0\x00\xe0"
      "\x00"                                                                            },
    // '5'
    { 0,  12, 12,
     "\xe0\xff\xe0\xff\x00\xe0\x00\xe0\xc0\xff\xe0\xff\xf0\x00\x70\x00\x70\x00\xf0\xf0\xe0\x7f\xc0"
      "\x3f"                                                                            },
    // '6'
    { 0,  12, 12,
     "\xc0\x3f\xe0\x7f\xe0\xf0\x00\xe0\xc0\xef\xe0\xff\xf0\xf0\x70\xe0\x70\xe0\xf0\xf0\xe0\x7f\xc0"
      "\x3f"                                                                            },
    // '7'
    { 0,  12, 12,
     "\xf0\xff\xf0\xff\x70\xe0\x70\x00\x70\x00\xe0\x00\xc0\x01\x80\x03\x00\x07\x00\x0e\x00\x0e\x00"
      "\x0e"                                                                            },
    // '8'
    { 0,  12, 12,
     "\xc0\x3f\xe0\x7f\xe0\x79\xe0\x70\xe0\x79\xc0\x3f\xe0\x7f\xf0\xf0\x70\xe0\xf0\xf0\xe0\x7f\xc0"
      "\x3f"                                                                            },
    // '9'
    { 0,  12, 12,
     "\xc0\x3f\xe0\x7f\xf0\xf0\x70\xe0\x70\xe0\xf0\xf0\xf0\x7f\x70\x3f\x70\x00\xf0\x70\xe0\x7f\xc0"
      "\x3f"                                                                            },
    // ':'
    { 4,  8,  4,  "\xf0\xf0\x00\x00\x00\x00\xf0\xf0"                                    },
    // ';'
    { 4,  10, 4,  "\xf0\xf0\x00\x00\x00\x00\xf0\xf0\x70\xe0"                            },
    // '<'
    { 0,  12, 9,
     "\x80\x07\x00\x0f\x00\x1e\x00\x3c\x00\x78\x00\xf0\x00\xf0\x00\x78\x00\x3c\x00\x1e\x00\x0f\x80"
      "\x07"                                                                            },
    // '='
    { 4,  6,  12, "\xf0\xff\xf0\xff\x00\x00\x00\x00\xf0\xff\xf0\xff"                    },
    // '>'
    { 0,  12, 9,
     "\x00\xf0\x00\x78\x00\x3c\x00\x1e\x00\x0f\x80\x07\x80\x07\x00\x0f\x00\x1e\x00\x3c\x00\x78\x00"
      "\xf0"                                                                            },
    // '?'
    { 0,  12, 11,
     "\x80\x7f\xc0\xff\xe0\xe1\xe0\x00\xe0\x01\x80\x07\x00\x1e\x00\x1c\x00\x1c\x00\x00\x00\x1c\x00"
      "\x1c"                                                                            },
    // '@'
    { 0,  12, 13,
     "\xe0\x3f\xf0\x7f\x78\xf0\xf8\xe3\xf8\xe7\x38\xe7\x38\xe7\xf8\xe7\xf8\xe3\x00\xf0\xf0\x7f\xe0"
      "\x3f"                                                                            },
    // 'A'
    { 0,  12, 13,
     "\x00\x07\x00\x07\x80\x0f\x80\x0f\xc0\x1d\xc0\x1d\xe0\x38\xe0\x3f\xf0\x7f\x70\x70\x38\xe0\x38"
      "\xe0"                                                                            },
    // 'B'
    { 0,  12, 13,
     "\xc0\xff\xe0\xff\xf0\xe0\x70\xe0\xe0\xe0\xc0\xff\xf0\xff\x78\xe0\x38\xe0\x78\xe0\xf0\xff\xe0"
      "\xff"                                                                            },
    // 'C'
    { 0,  12, 14,
     "\xf0\x3f\xf8\x7f\x3c\xf0\x1c\xe0\x1c\xe0\x00\xe0\x00\xe0\x1c\xe0\x1c\xe0\x3c\xf0\xf8\x7f\xf0"
      "\x3f"                                                                            },
    // 'D'
    { 0,  12, 12,
     "\xc0\xff\xe0\xff\xf0\xe0\x70\xe0\x70\xe0\x70\xe0\x70\xe0\x70\xe0\x70\xe0\xf0\xe0\xe0\xff\xc0"
      "\xff"                                                                            },
    // 'E'
    { 0,  12, 11,
     "\xe0\xff\xe0\xff\x00\xe0\x00\xe0\x00\xe0\x80\xff\x80\xff\x00\xe0\x00\xe0\x00\xe0\xe0\xff\xe0"
      "\xff"                                                                            },
    // 'F'
    { 0,  12, 11,
     "\xe0\xff\xe0\xff\x00\xe0\x00\xe0\x00\xe0\x80\xff\x80\xff\x00\xe0\x00\xe0\x00\xe0\x00\xe0\x00"
      "\xe0"                                                                            },
    // 'G'
    { 0,  12, 14,
     "\xf0\x3f\xf8\x7f\x3c\xf0\x1c\xe0\x00\xe0\x00\xe0\xfc\xe1\xfc\xe1\x1c\xe0\x3c\xf0\xfc\x7f\xec"
      "\x3f"                                                                            },
    // 'H'
    { 0,  12, 13,
     "\x38\xe0\x38\xe0\x38\xe0\x38\xe0\x38\xe0\xf8\xff\xf8\xff\x38\xe0\x38\xe0\x38\xe0\x38\xe0\x38"
      "\xe0"                                                                            },
    // 'I'
    { 0,  12, 3,  "\xe0\xe0\xe0\xe0\xe0\xe0\xe0\xe0\xe0\xe0\xe0\xe0"                    },
    // 'J'
    { 0,  12, 12,
     "\x70\x00\x70\x00\x70\x00\x70\x00\x70\x00\x70\x00\x70\x00\x70\x00\x70\xe0\xf0\xf0\xe0\x7f\xc0"
      "\x3f"                                                                            },
    // 'K'
    { 0,  12, 12,
     "\x70\xe0\xe0\xe0\xc0\xe1\x80\xe3\x00\xe7\x00\xee\x00\xff\x80\xfb\xc0\xf1\xe0\xe0\x70\xe0\x70"
      "\xe0"                                                                            },
    // 'L'
    { 0,  12, 11,
     "\x00\xe0\x00\xe0\x00\xe0\x00\xe0\x00\xe0\x00\xe0\x00\xe0\x00\xe0\x00\xe0\x00\xe0\xe0\xff\xe0"
      "\xff"                                                                            },
    // 'M'
    { 0,  12, 15,
     "\x1e\xf0\x1e\xf0\x3e\xf8\x3e\xf8\x7e\xfc\x7e\xfc\xee\xee\xee\xee\xce\xe7\xce\xe7\x8e\xe3\x8e"
      "\xe3"                                                                            },
    // 'N'
    { 0,  12, 13,
     "\x38\xe0\x38\xf0\x38\xf8\x38\xfc\x38\xfe\x38\xef\xb8\xe7\xf8\xe3\xf8\xe1\xf8\xe0\x78\xe0\x38"
      "\xe0"                                                                            },
    // 'O'
    { 0,  12, 14,
     "\xf0\x3f\xf8\x7f\x3c\xf0\x1c\xe0\x1c\xe0\x1c\xe0\x1c\xe0\x1c\xe0\x1c\xe0\x3c\xf0\xf8\x7f\xf0"
      "\x3f"                                                                            },
    // 'P'
    { 0,  12, 13,
     "\xe0\xff\xf0\xff\x78\xe0\x38\xe0\x78\xe0\xf0\xff\xe0\xff\x00\xe0\x00\xe0\x00\xe0\x00\xe0\x00"
      "\xe0"                                                                            },
    // 'Q'
    { 0,  12, 15,
     "\xe0\x3f\xf0\x7f\x78\xf0\x38\xe0\x38\xe0\x38\xe0\x38\xe0\xb8\xe3\xf8\xe1\xf8\xf0\xfc\x7f\xee"
      "\x3f"                                                                            },
    // 'R'
    { 0,  12, 13,
     "\xe0\xff\xf0\xff\x78\xe0\x38\xe0\x78\xe0\xf0\xff\xe0\xff\xf0\xe0\x78\xe0\x38\xe0\x38\xe0\x38"
      "\xe0"                                                                            },
    // 'S'
    { 0,  12, 12,
     "\xc0\x3f\xe0\x7f\xf0\xf0\x70\xe0\x00\x78\x00\x1e\x80\x07\xe0\x01\x70\xe0\xf0\xf0\xe0\x7f\xc0"
      "\x3f"                                                                            },
    // 'T'
    { 0,  12, 11,
     "\xe0\xff\xe0\xff\x00\x0e\x00\x0e\x00\x0e\x00\x0e\x00\x0e\x00\x0e\x00\x0e\x00\x0e\x00\x0e\x00"
      "\x0e"                                                                            },
    // 'U'
    { 0,  12, 13,
     "\x38\xe0\x38\xe0\x38\xe0\x38\xe0\x38\xe0\x38\xe0\x38\xe0\x38\xe0\x38\xe0\x78\xf0\xf0\x7f\xe0"
      "\x3f"                                                                            },
    // 'V'
    { 0,  12, 13,
     "\x38\xe0\x38\xe0\x70\x70\x70\x70\xe0\x38\xe0\x38\xc0\x1d\xc0\x1d\x80\x0f\x80\x0f\x00\x07\x00"
      "\x07"                                                                            },
    // 'W'
    { 0,  12, 15,
     "\x8e\xe3\x8e\xe3\x8e\xe3\x8e\xe3\xdc\x77\xdc\x77\xdc\x77\xfc\x7e\xf8\x3e\x78\x3c\x78\x3c\x38"
      "\x38"                                                                            },
    // 'X'
    { 0,  12, 15,
     "\x1e\xf0\x3c\x78\x78\x3c\xf0\x1e\xe0\x0f\xc0\x07\xc0\x07\xe0\x0f\xf0\x1e\x78\x3c\x3c\x78\x1e"
      "\xf0"                                                                            },
    // 'Y'
    { 0,  12, 15,
     "\x1e\xf0\x3c\x78\x78\x3c\xf0\x1e\xe0\x0f\xc0\x07\x80\x03\x80\x03\x80\x03\x80\x03\x80\x03\x80"
      "\x03"                                                                            },
    // 'Z'
    { 0,  12, 13,
     "\xf8\xff\xf8\xff\xf0\x00\xe0\x01\xc0\x03\x80\x07\x00\x0f\x00\x1e\x00\x3c\x00\x78\xf8\xff\xf8"
      "\xff"                                                                            },
};

/* 1d7d:2962 : 2 bytes */
std::int16_t g_textSpacing = 1;

/* 1b06:1552 */
void drawText(std::int16_t x, std::int16_t y, const char* s, Color color)
{
    std::uint8_t oldGlyphHeight = g_glyphHeight;
    for (; *s; ++s) {
        std::uint8_t shiftedChar = *s - 0x20;
        const struct GlyphInfo* g = &g_glyphs[shiftedChar];
        g_glyphHeight = g->height;
        std::int16_t tileY = g->verticalOffset + y;
        if (g->width <= 8)
            drawGlyphW8(
                /* FIXME!!! regenerate constants, use uint8_t instead of char */ (std::uint8_t*)
                    g->glyph,
                x, tileY, color);
        else
            drawGlyphW16(
                /* FIXME!!! regenerate constants, use uint8_t instead of char */ (std::uint8_t*)
                    g->glyph,
                x, tileY, color);
        x += g->width + g_textSpacing;
    }
    g_glyphHeight = oldGlyphHeight;
}

/* 1b06:077c */
void drawTextSmall(std::int16_t, std::int16_t, const char*, Color)
{
    // TODO implement
}

} // namespace resl
