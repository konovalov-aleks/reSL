#pragma once

#include <cstdint>

namespace resl {

struct RailInfo2 {
    std::uint8_t railType;
    std::int8_t tileDX;
    std::int8_t tileDY;
    std::uint8_t _type1;
    std::uint8_t _type2;
    std::uint8_t unknown1;
};

/* 1d7d:2402 : 216 bytes */
extern RailInfo2 x_railInfo[6][6]; /* [railType][?] */

} // namespace resl
