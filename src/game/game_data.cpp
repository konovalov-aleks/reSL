#include "game_data.h"

namespace resl {

/* 262d:21d0 : 2 byte */
std::uint16_t g_railsLoadedOffset;

/* 262d:21ce : 2 byte */
std::uint16_t g_entrancesLoadedOffset;

/* 1d7d:01e8 : 20 byte */
char g_playerName[20];

/* 262d:5eff : 1 byte */
bool g_isDemoMode = false;

/* 262d:6f4f : 1 byte */
bool g_gameOver = false;

} // namespace resl
