#pragma once

#include <cstdint>

namespace resl {

/* 262d:6ef9 : 1 byte */
extern std::uint8_t g_lastKeyPressed;

/* 262d:6f11 : 1 byte */
extern std::uint8_t g_lastKeyCode;

//-----------------------------------------------------------------------------

// The original game uses INT9 interruption to listen keyboard actions.
// Thus, this function in the original game receives nothing - it reads the
// data from the IO port 0x60.
/* 14af:06e7 */
void keyboardInteruptionHandler(std::uint8_t keycode);

} // namespace resl
