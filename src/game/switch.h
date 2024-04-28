#pragma once

#include "types/rail_info.h"
#include "types/switch.h"

namespace resl {

/* 1ad3:000c */
void createSwitches(RailInfo&);

/* 19de:00ec */
void toggleSwitch(Switch&);

} // namespace resl
