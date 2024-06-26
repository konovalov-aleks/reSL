#pragma once

#include <cstdint>

namespace resl {

// http://www.techhelpmanual.com/106-int_09h__keyboard_interrupt.html

constexpr std::uint8_t g_keyReleasedFlag = 0x80;

using KeyboardHandler = void (*)(std::uint8_t keycode);

} // namespace resl
