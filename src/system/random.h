#pragma once

#include <cstdint>

namespace resl {

/* 1ca7:000a */
[[nodiscard]] std::int16_t genRandomNumber(std::int16_t max);

/* 1594:0091 */
[[nodiscard]] std::int16_t symmetricRand(std::int16_t max);

} // namespace resl
