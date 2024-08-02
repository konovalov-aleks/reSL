#pragma once

#include "mode.h"
#include <game/rail_info.h>

namespace resl::mouse {

/* 1d7d:1ca8 : 6 bytes */
extern RailInfo g_railCursorState;

/* 1d7d:1cce : 32 bytes */
extern Mode g_modeConstruction;

} // namespace resl::mouse
