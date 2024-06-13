#pragma once

#include <system/time.h>

#include <cassert>
#include <chrono>
#include <coroutine> // IWYU pragma: export
#include <cstdlib>
#include <optional>

namespace resl {

struct TaskPromise;

struct Task : public std::coroutine_handle<TaskPromise> {
    using Time = std::chrono::steady_clock::time_point;

    using promise_type = TaskPromise;
};

struct TaskPromise {
    Task get_return_object() { return { Task::from_promise(*this) }; }
    std::suspend_never initial_suspend() noexcept { return {}; }
    std::suspend_always final_suspend() noexcept { return {}; }
    void return_void() { }
    void unhandled_exception() { std::abort(); }

    std::optional<Task::Time> m_sleepUntil;
    bool m_suspended = false;
};

class SleepAwaitable {
public:
    SleepAwaitable() = default;

    template <typename DurationT>
    SleepAwaitable(DurationT sleepFor)
        : m_awakeTime(std::chrono::steady_clock::now() + sleepFor)
    {
    }

    bool await_ready() { return false; }
    void await_suspend(std::coroutine_handle<TaskPromise>);
    void await_resume() { }

private:
    std::optional<Task::Time> m_awakeTime;
};

void addTask(Task&&);

inline SleepAwaitable yield()
{
    // Here we could instantly switch to another task ('return {}').
    // But in case of this game this makes no sense - we have no tasks that
    // need to be called as often as possible.
    // So, make a small sleep here to reduce the CPU overhead.
    return { std::chrono::milliseconds(1) };
}

inline SleepAwaitable sleep(TimeT time)
{
    assert(time > 0);
    return { std::chrono::milliseconds(time * MsPerTick) };
}

void runScheduler();
void stopScheduler();

} // namespace resl
