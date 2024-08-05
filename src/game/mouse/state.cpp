#include "state.h"

#include "construction_mode.h"
#include "management_mode.h"
#include "mode.h"

#include <system/driver/driver.h>

namespace resl::mouse {

/* 262d:6f02 : 12 bytes */
State g_state = {};

//-----------------------------------------------------------------------------

/* 14af:0104 */
void setMode(Mode& newMode)
{
    if (g_state.mode)
        g_state.mode->clearFn();
    g_state.mode = &newMode;
    Driver::instance().mouse().setCursorVisibility(g_state.mode == &g_modeManagement);
    newMode.drawFn();
}

void toggleMode()
{
    setMode(g_state.mode == &g_modeManagement ? g_modeConstruction
                                              : g_modeManagement);
}

} // namespace resl::mouse
