#include "active_sleep.h"

#include <system/driver/driver.h>

#include <cstdint>

namespace resl {

// The original game performes a measurement of how many iterations need to be
// performed on the current hardware to spend 1 ms. This information is used in
// the active sleep implementation.
// At least in the first steps, I don't see any reason to implement this logic
// and will use std::this_thread::sleep_for instead.

/* 1d7d:1e08 : 4 bytes */
// std::int32_t g_systemPerfMetrics = 200000;

/* 1594:00ab */
void calibrateActiveSleep()
{
    // the original game performes 200000 loop iterations and measure the time here
}

/* 1594:0113 */
void activeSleep(std::int16_t ticks)
{
    // 1 tick ~= 1 ms
    Driver::instance().sleep(ticks);
}

} // namespace resl
