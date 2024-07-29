#pragma once

#include <cassert>
#include <concepts>

namespace resl {

// The portable implementation of X86 SAR instruction
template <std::signed_integral T>
T sar(T x, unsigned count)
{
    assert(count <= sizeof(T) * 8);
    T s = -(x < 0);
    return ((s ^ x) >> count) ^ s;
}

} // namespace resl
