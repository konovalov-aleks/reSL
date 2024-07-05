#pragma once

#include "types/rail_info.h"
#include <tasks/message_queue.h>
#include <tasks/task.h>

namespace resl {

/* The original game uses an own implementation of stack-based coroutines.
   I decided not to restore the sources of this mechanic, because it's
   non-portable and heavily dependent on X86 architecture.

   So, the address below contains a different data structure, but the
   meaning is roughly the same. */

/* 1d7d:1c96 */
extern MessageQueue<RailInfo> g_railConstructionMsgQueue;

/* 1d7d:20f8 */
extern Task g_taskRoadConstruction;

//-----------------------------------------------------------------------------

/* 17bf:000d */
Task taskRoadConstruction();

} // namespace resl
