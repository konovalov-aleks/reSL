#pragma once

#include <cstdint>

namespace resl {

// The timer in the original game has a frequency ~100Hz
inline constexpr int MsPerTick = 10;

using TimeT = std::uint16_t;

/* 1c61:000e */
TimeT getTime();

/* 1594:000e */
void initTimer();

/* 16a6:05eb */
void disableTimer();

/* 16a6:0600 */
void enableTimer();

} // namespace resl
