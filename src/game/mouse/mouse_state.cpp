#include "mouse_state.h"

#include "mouse_mode.h"

#include <cassert>

namespace resl::mouse {

/* 262d:6f02 : 12 bytes */
MouseState g_state = {};

//-----------------------------------------------------------------------------

/* 14af:0104 */
void setMouseMode(MouseMode& newMode)
{
    assert(g_state.mode);
    g_state.mode->clearFn();
    g_state.mode = &newMode;
    newMode.drawFn();
}

} // namespace resl::mouse
