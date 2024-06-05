#include "mouse_state.h"

#include "management_mode.h"

namespace resl::mouse {

/* 262d:6f02 : 12 bytes */
const MouseState g_mouseState = {
    &g_modeManagement
    /* TODO initialize */
};

} // namespace resl::mouse
