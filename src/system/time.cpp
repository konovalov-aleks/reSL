#include "time.h"

#include <SDL_stdinc.h>
#include <SDL_timer.h>

#include <atomic>

namespace resl {

// Of course, the original game is single-threaded, they use interruptions to
// update this value => probably, this value is just marked as volatile there
// (This is enough on single-core systems).
// But for simplicity, I use SDL timer to simulate interruptions, it uses
// a separate thread to call a callback.
// Thus, I need to take care about synchronization => must to use atomic here
/* 1c61:0014 */
static std::atomic<TimeT> g_clock = 0;

/* 1c61:000e */
std::uint16_t getTime()
{
    return g_clock.load(std::memory_order_relaxed);
}

static Uint32 sdlTimerCallback(Uint32 interval, void* /* param */)
{
    g_clock.fetch_add(1, std::memory_order_relaxed);
    return interval;
}

void startTimer()
{
    /* The original game initialized the timer here:
            1594:000e

       It sets the timer with the following parameters:
            channel: 1
            access mode: hibyte only
            mode: 2 (rate generator)

        It passes the following devider:
            (10000 << 16) / 54945

        So the frequency is:
            1.193182 MHz. / ((10000 << 16) / 54945) ~~> 100Hz
    */

    /* The original game sets the interruption vector here:
            1000:1330

       Then it configures the timer frequency:
            1594:000e

        And finally it calibrates the timer:
            1594:00ab

        So, this code is very specific for X86 DOS
        => we will use SDL timer instead
     */
    SDL_AddTimer(MsPerTick, sdlTimerCallback, nullptr);
}

} // namespace resl
