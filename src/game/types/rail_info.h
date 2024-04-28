#pragma once

#include <cstdint>

namespace resl {

struct RailInfo {
    std::uint8_t roadTypeMask;
    std::uint8_t tileX;
    std::uint8_t tileY;
    std::uint8_t railType;
    /* cur year - 8 */
    std::uint8_t year_8;
    std::uint8_t unknown1;
};

static_assert(sizeof(RailInfo) == 0x6);

} // namespace resl
