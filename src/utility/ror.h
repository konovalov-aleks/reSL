#pragma once

#include <cassert>
#include <concepts>

namespace resl {

// The portable implementation of X86 ROR instruction
template <std::unsigned_integral T>
constexpr T ror(T x, unsigned count)
{
    constexpr auto nBits = sizeof(T) * 8;
    assert(count <= nBits);
    return (x >> count) | (x << (nBits - count));
}

} // namespace resl
