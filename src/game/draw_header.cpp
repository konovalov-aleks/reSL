#include "draw_header.h"

#include <graphics/drawing.h>
#include <graphics/text.h>

namespace resl {

/* 1d7d:0138 : 48 bytes */
static const char* const s_alphabet[12] = { "9", "0", "1", "2", "3", "4",
                                            "5", "6", "7", "8", "9", "0" };

/* 12c5:0342 */
void drawHeaderFieldFontTexture()
{
    drawing::filledRectangle(680, 397, 2, 192, 0xFF, Color::White);
    drawing::filledRectangle(680, 397, 1, 192, 0x80, Color::DarkGray);
    for (int i = 0; i < 12; ++i)
        drawText(682, i * 16 + 399, s_alphabet[i], Color::Black);
}

} // namespace resl
