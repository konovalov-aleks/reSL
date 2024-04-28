#pragma once

#include <cstdint>

namespace resl {

struct HeaderField {
    std::int16_t x;
    std::int16_t y;
    std::int16_t _valueLimit;
    std::int8_t nDigits;
    std::int8_t curAnimatingDigit;
    std::int16_t value;
    std::int16_t _counter;
    std::uint8_t digitValues[6];
};
static_assert(sizeof(HeaderField) == 0x12);

struct Headers {
    HeaderField trains;
    HeaderField money;
    HeaderField year;
    HeaderField level;
};
static_assert(sizeof(Headers) == 0x48);

} // namespace resl
