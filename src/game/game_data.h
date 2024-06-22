#pragma once

#include <cstdint>

namespace resl {

/* 262d:21d0 : 2 byte */
extern std::uint16_t g_railsLoadedOffset;

/* 262d:21ce : 2 byte */
extern std::uint16_t g_entrancesLoadedOffset;

/* 1d7d:01e8 : 20 byte */
extern char g_playerName[20];

} // namespace resl
