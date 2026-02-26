#include "time.h"

#include "driver/driver.h"

#include <cassert>
#include <chrono>
#include <optional>

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

        It passes the following divider:
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
            std::chrono::duration_cast<std::chrono::milliseconds>(now - m_startTime).count();
        return static_cast<std::uint16_t>(m_startValue + ms * m_multiplier / MsPerTick);
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

    int timeAcceleration() const
    {
        return m_multiplier;
    }

    void setTimeAcceleration(int multiplier)
    {
        stop();
        m_multiplier = multiplier;
        start();
    }

private:
    Timer()
    {
        Driver::instance().vga().addOverlay([this](SDL_Renderer* r, int) {
            Texture* texture = nullptr;
            ;
            switch (m_multiplier) {
            case 2:
                if (!m_textureFast2)
                    m_textureFast2 = Texture(r, "time_fast2.png");
                texture = &*m_textureFast2;
                break;
            case 3:
                if (!m_textureFast3)
                    m_textureFast3 = Texture(r, "time_fast3.png");
                texture = &*m_textureFast3;
                break;
            default:
                return;
            }

            int texW = 0;
            int texH = 0;
            SDL_QueryTexture(*texture, nullptr, nullptr, &texW, &texH);

            static constexpr int MARGIN_X = 20;
            static constexpr int MARGIN_Y = 50;

            assert(texture);
            SDL_Rect dst;
            dst.w = texW;
            dst.h = texH;
            dst.x = MARGIN_X;
            dst.y = SCREEN_HEIGHT - MARGIN_Y;
            SDL_RenderCopy(r, *texture, nullptr, &dst);
        });
    }

    Timer(const Timer&) = delete;
    Timer& operator=(const Timer&) = delete;

    std::optional<Texture> m_textureFast2;
    std::optional<Texture> m_textureFast3;

    TimeT m_startValue = 0;
    ClockT::time_point m_startTime;
    int m_multiplier = 1;
    bool m_enabled = false;
};

/* 1c61:000e */
std::uint16_t getTime()
{
    // The original game just reads the timer value here from the memory.
    return Timer::instance().time();
}

/* 1594:000e */
void initTimer()
{
    // The original game configures the timer frequency here.
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

void setTimeAcceleration(int multiplier)
{
    Timer::instance().setTimeAcceleration(multiplier);
}

int getTimeAcceleration()
{
    return Timer::instance().timeAcceleration();
}

} // namespace resl
