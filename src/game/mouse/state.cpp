#include "state.h"

#include "mode.h"

#include <cassert>

namespace resl::mouse {

/* 262d:6f02 : 12 bytes */
State g_state = {};

//-----------------------------------------------------------------------------

/* 14af:0104 */
void setMode(Mode& newMode)
{
    assert(g_state.mode);
    g_state.mode->clearFn();
    g_state.mode = &newMode;
    newMode.drawFn();
}

} // namespace resl::mouse
