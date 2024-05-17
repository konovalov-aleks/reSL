#pragma once

#include "impl/driver_sdl.h"

namespace resl {

void graphics_init(bool debugMode = false);
void graphics_close();
void graphics_update();

bool poll_event();

} // namespace resl
