#pragma once

#include <game/header_field.h>

#include <cstdint>

namespace resl {

static constexpr std::int16_t g_headerHeight = 47;

//-----------------------------------------------------------------------------

/* 12c5:0342 */
void drawHeaderFieldFontTexture();

/* 132d:0086 */
void drawHeaderBackground(std::int16_t yOffset);

/* 12c5:01d7 */
void drawHeaderField(const HeaderField&);

/* 12c5:02d1 */
void drawHeaderData(std::int16_t trains, std::int16_t money, std::int16_t year,
                    std::int16_t level, std::int16_t yOffset);

/* 137c:0450 */
void drawDispatcher(std::int16_t entranceIdx, bool signalling);

/* 137c:04c3 */
void drawDispatchers(std::int16_t yOffset);

} // namespace resl
