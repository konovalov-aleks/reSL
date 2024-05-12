#pragma once

#include <system/time.h>

#include <coroutine>

#include <cassert>
#include <cstdlib>

namespace resl {

struct TaskPromise;

struct Task : public std::coroutine_handle<TaskPromise> {
    using promise_type = TaskPromise;
};

struct TaskPromise {
    Task get_return_object() { return { Task::from_promise(*this) }; }
    std::suspend_never initial_suspend() noexcept { return {}; }
    std::suspend_always final_suspend() noexcept { return {}; }
    void return_void() { }
    void unhandled_exception() { std::abort(); }

    TimeT m_sleepUntil = 0;
};

class SleepAwaitable {
public:
    SleepAwaitable()
        : m_awakeTime(0)
    {
    }
    SleepAwaitable(TimeT sleepFor)
        : m_awakeTime(getTime() + sleepFor)
    {
    }

    bool await_ready() { return false; }
    void await_suspend(std::coroutine_handle<TaskPromise>);
    void await_resume() { }

private:
    TimeT m_awakeTime;
};

void addTask(Task&&);

inline SleepAwaitable yield()
{
    return {};
}

inline SleepAwaitable sleep(TimeT time)
{
    assert(time > 0);
    return { time };
}

void runScheduler();
void stopScheduler();

} // namespace resl
