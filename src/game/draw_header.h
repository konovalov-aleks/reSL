#pragma once

#include <cstdint>

namespace resl {

/* 12c5:0342 */
void drawHeaderFieldFontTexture();

/* 132d:0086 */
void drawHeaderBackground(std::int16_t yOffset);

/* 12c5:02d1 */
void drawHeaderData(std::int16_t trains, std::int16_t money, std::int16_t year,
                    std::int16_t level, std::int16_t yOffset);

} // namespace resl
