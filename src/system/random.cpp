#include "random.h"

#include <cstdlib>

namespace resl {

/* 1ca7:000a */
std::int16_t genRandomNumber(std::int16_t max)
{
    return std::rand() % max;
}

/* 1594:0091 */
std::int16_t symmetricRand(std::int16_t max)
{
    return max - genRandomNumber(max * 2 - 1);
}

} // namespace resl
