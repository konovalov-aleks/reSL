#pragma once

#include "io_status.h"

#include <cstdint>

namespace resl {

/* 1400:009c */
IOStatus loadSavedGame(const char* fileName);

/* 174e:052b */
std::int16_t readLevel();

} // namespace resl
