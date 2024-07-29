#pragma once

#include "mode.h"

namespace resl::mouse {

/* 1d7d:1cae : 32 bytes */
extern Mode g_modeManagement;

//-----------------------------------------------------------------------------

/* 14af:0089 */
void drawArrowCursor();

/* 14af:00e6 */
void eraseArrowCursor();

} // namespace resl::mouse
