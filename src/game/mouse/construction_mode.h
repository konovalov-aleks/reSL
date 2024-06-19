#pragma once

#include "mouse_mode.h"
#include <game/types/rail_info.h>

namespace resl::mouse {

/* 1d7d:1ca8 : 6 bytes */
extern RailInfo g_railCursorState;

/* 1d7d:1cce : 32 bytes */
extern MouseMode g_modeConstruction;

} // namespace resl::mouse
