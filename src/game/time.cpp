#include "time.h"

#include <ctime>

namespace resl {

/* 1c61:000e */
std::uint16_t getTime() { return static_cast<std::uint16_t>(std::clock() / (CLOCKS_PER_SEC / 10)); }

} // namespace resl
