#pragma once

#include <cstdint>

namespace resl {

static constexpr std::int16_t g_footerYPos = 334;

//-----------------------------------------------------------------------------

/* 12ba:0097 */
void drawCopyright(std::int16_t yOffset);

/* 132d:013c */
void drawStatusBarWithCopyright(std::int16_t yOffset);

/* 12ba:0003 */
void showStatusMessage(const char* msg, std::int16_t yOffset = 0);

/* 12ba:005c */
void updateStatusBar();

} // namespace resl
