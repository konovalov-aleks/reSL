#include "buffer.h"

namespace resl {

static std::uint8_t data[0xFFFF];

/* 262d:21d8 */
std::uint8_t* g_pageBuffer = data;

} // namespace resl
