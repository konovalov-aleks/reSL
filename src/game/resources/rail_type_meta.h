#pragma once

#include <game/types/semaphore.h>

#include <cstdint>

namespace resl {

struct RailTypeMeta {
    std::int8_t tileOffsetX;
    std::int8_t tileOffsetY;
    std::uint8_t railType;
    SemaphoreType semaphoreType;
};

/* 1d5d:0000 : 48 bytes */
extern const RailTypeMeta g_railTypeMeta[12];

} // namespace resl
