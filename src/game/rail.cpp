#include "rail.h"

namespace resl {

/* 262d:259a : 11880 bytes */
Rail g_rails[11][11][6]; // [tileX][tileY][railType]

/* 262d:6ef4 : 4 bytes */
const RailConnection g_emptyRailConnection = { nullptr, 0 };

} // namespace resl
