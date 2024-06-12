#pragma once

#include <game/train.h>

#include <cstdint>

namespace resl {

struct TrainSpecification {
    std::int16_t minYear;
    std::int16_t maxYear;
    std::uint8_t maxSpeed;
    CarriageType possibleCarriages[5];
};

//-----------------------------------------------------------------------------

/* 1d32:0000 : 140 bytes */
extern const TrainSpecification g_trainSpecifications[14];

} // namespace resl
