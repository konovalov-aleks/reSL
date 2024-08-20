#pragma once

#include <cstdint>

namespace resl {

// http://www.techhelpmanual.com/106-int_09h__keyboard_interrupt.html
inline constexpr std::uint8_t g_keyReleasedFlag = 0x80;

using KeyboardHandler = void (*)(std::uint8_t keycode);

// http://www.techhelpmanual.com/57-keyboard_scan_codes.html
inline constexpr std::uint8_t g_keyEscape = 1;
inline constexpr std::uint8_t g_keySpace = 57;

//-----------------------------------------------------------------------------

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

/* 1c71:000f */
void updateKeyboardLeds(std::int16_t);

} // namespace resl
