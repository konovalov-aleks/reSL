#include "state.h"

#include "construction_mode.h"
#include "management_mode.h"
#include "mode.h"
#include <game/field_tile_grid_overlay.h>

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
    GridOverlay::instance().setVisibility(g_state.mode == &g_modeConstruction);
    newMode.drawFn();
}

void toggleMode()
{
    setMode(g_state.mode == &g_modeManagement ? g_modeConstruction
                                              : g_modeManagement);
}

} // namespace resl::mouse
