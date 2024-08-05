#pragma once

#include "io_status.h"
#include <tasks/task.h>

namespace resl {

/* 262d:5eff : 1 byte */
extern bool g_isDemoMode;

/* 1d7d:2114 : 14 bytes */
extern Task g_taskDemoAI;

//-----------------------------------------------------------------------------

/* 16a6:09f8 */
IOStatus loadDemo();

/* 16a6:0a7b */
void stopDemo();

/* 1300:0003 */
Task taskDemoAI();

} // namespace resl
