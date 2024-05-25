#include "active_sleep.h"

#include "driver/driver.h"

#include <chrono>
#include <cstdint>
#include <thread>

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
void activeSleep(std::int16_t ms)
{
    std::this_thread::sleep_for(std::chrono::milliseconds(ms));

    // activeSleep is often used to implement animation.
    // The VGA hardware draws the picture independently of the program code flow
    // => it will draw while the game is sleeping.
    // But reSL implementation is single-threaded => we have to explicitly call
    // vga().flush() to update the image.
    Driver::instance().vga().flush();
}

} // namespace resl
