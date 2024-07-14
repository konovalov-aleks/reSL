#include "time.h"

#include <chrono>

namespace resl {

/* The original game stores the time value in a global unsigned integer
   value (1c61:0014, 2 bytes) and increments it in the timer interruption
   handler (1c61:002e).

   So, this code is very specific for X86 DOS => we will use an
   alternative portable implementation instead.

   Code locations in the original game:
       1. It sets the interruption vector here:
            1000:1330

       2. Then it configures the timer frequency:
            1594:000e

        The timer is initialized with the following parameters:
            channel: 1
            access mode: hibyte only
            mode: 2 (rate generator)

        It passes the following devider:
            (10000 << 16) / 54945

        So the frequency is:
            1.193182 MHz. / ((10000 << 16) / 54945) ~~> 100Hz

        3. And finally it calibrates the timer:
            1594:00ab
 */

class Timer {
    using ClockT = std::chrono::steady_clock;

public:
    static Timer& instance()
    {
        static Timer g_instance;
        return g_instance;
    }

    std::uint16_t time()
    {
        if (!m_enabled)
            return m_startValue;

        ClockT::time_point now = ClockT::now();
        const auto ms =
            std::chrono::duration_cast<std::chrono::milliseconds>(now - m_startTime);
        return m_startValue + ms.count() / MsPerTick;
    }

    void start()
    {
        if (!m_enabled) {
            m_startTime = ClockT::now();
            m_enabled = true;
        }
    }

    void stop()
    {
        if (m_enabled) {
            m_startValue = time();
            m_enabled = false;
        }
    }

private:
    Timer() = default;
    Timer(const Timer&) = delete;
    Timer& operator=(const Timer&) = delete;

    TimeT m_startValue = 0;
    ClockT::time_point m_startTime;
    bool m_enabled = false;
};

/* 1c61:000e */
std::uint16_t getTime()
{
    // the original game just reads the timer value here from the memory
    return Timer::instance().time();
}

/* 1594:000e */
void initTimer()
{
    // the original game configures the timer frequency here
    Timer::instance().start();
}

/* 16a6:05eb */
void disableTimer()
{
    // In the original game, the INT8 vector is replaced with an empty handler.
    Timer::instance().stop();
}

/* 16a6:0600 */
void enableTimer()
{
    // In the original game, the INT8 vector is set here.
    Timer::instance().start();
}

} // namespace resl
