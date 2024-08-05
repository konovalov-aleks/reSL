#pragma once

#include "header_field.h"
#include <tasks/message_queue.h>
#include <tasks/task.h>

#include <cstdint>

namespace resl {

struct HeaderValueChangeRequest {
    std::int8_t delta;
    HeaderFieldId fieldId;
    // if delta is zero, the value will be set without animation
    std::int16_t value;
};

//-----------------------------------------------------------------------------

/* The original game uses an own implementation of stack-based coroutines.
   I decided not to restore the sources of this mechanic, because it's
   non-portable and heavily dependent on X86 architecture.

   So, the address below contains a different data structure, but the
   meaning is roughly the same.
*/
/* 1d7d:00de : 18 bytes */
extern MessageQueue<HeaderValueChangeRequest> g_headerAnimationTaskQueue;

/* 1d7d:20dc */
extern Task g_taskHeaderFieldAnimation;

/* 1d7d:00f0 : 72 bytes */
extern Headers g_headers;

//-----------------------------------------------------------------------------

/* 12c5:031b */
void startHeaderFieldAnimation(HeaderFieldId, std::int16_t delta);

/* 19de:0490 */
void spendMoney(std::int16_t delta);

/* 12c5:0008 */
Task taskHeaderFieldAnimation();

} // namespace resl
