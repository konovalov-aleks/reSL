#pragma once

#include <cassert>
#include <concepts>

namespace resl {

// The portable implementation of X86 SAR instruction
template <std::signed_integral T>
T sar(T x, unsigned count)
{
    constexpr auto nBits = sizeof(T) * 8;
    assert(count <= nBits);
    T s = -(x < 0);
    return ((s ^ x) >> count) ^ s;
}

} // namespace resl
