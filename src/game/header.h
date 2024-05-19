#pragma once

#include "types/header_field.h"
#include <tasks/task.h>

#include <cstdint>

namespace resl {

/* 1d7d:00f0 : 72 bytes */
extern Headers g_headers;

/* 12c5:031b */
void startHeaderFieldAnimation(HeaderFieldId, std::int16_t delta);

/* 19de:0490 */
void spendMoney(std::int16_t delta);

/* 12c5:0008 */
Task taskHeaderFieldAnimation();

} // namespace resl
