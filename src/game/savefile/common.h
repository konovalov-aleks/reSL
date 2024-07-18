#pragma once

#include <cstddef>

namespace resl {

/* sizeof(Entrance) can be different, because we have different size of
 * pointer */
constexpr std::size_t fileEntranceInfoSize = 0x16;
/* offsetof(Entrance, rail) */
constexpr std::size_t fileEntranceRailOffset = 4;
/* sizeof(Rail) */
constexpr std::size_t fileRailSize = 0x12;
/* sizeof(g_rails) */
constexpr std::size_t fileRailArraySize = fileRailSize * 6 * 10 * 11;
/* sizeof(g_entrances) */
constexpr std::size_t fileEntrancesArraySize = 0x84;

} // namespace resl
