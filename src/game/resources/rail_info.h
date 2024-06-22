#pragma once

#include <cstdint>

namespace resl {

struct RailInfo2 {
    std::uint8_t railType;
    std::int8_t tileDX;
    std::int8_t tileDY;
    std::uint8_t slot1;
    std::uint8_t slot2;
    std::uint8_t _padding;
};

/* 1d7d:2402 : 216 bytes */
extern RailInfo2 x_railInfo[6][6]; /* [railType][railType] */

} // namespace resl
