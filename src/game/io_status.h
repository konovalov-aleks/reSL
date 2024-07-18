#pragma once

#include <cstdint>

namespace resl {

enum class IOStatus : std::int8_t {
    NoError = 0x0,
    OpenError = 0x1,
    CloseError = 0x3
};

/* 262d:21d2 : 1 byte */
extern IOStatus g_ioStatus;

} // namespace resl
