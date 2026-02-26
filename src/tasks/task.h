#pragma once

#include <system/time.h>

#include <cassert>
#include <chrono>
#include <coroutine> // IWYU pragma: export
#include <cstdlib>

namespace resl {

struct TaskPromise;

struct Context {
    using Time = std::chrono::steady_clock::time_point;

    Context(unsigned priority)
        : m_priority(priority)
    {
    }

    // a newly created task has a highest execution priority
    Time m_sleepUntil = {};
    unsigned m_priority;
    bool m_suspended = false;
};

struct Task : public std::coroutine_handle<TaskPromise> {
    using promise_type = TaskPromise;

    bool await_ready() { return false; }
    inline void await_suspend(std::coroutine_handle<TaskPromise> inner);
    void await_resume() { };
    inline void resume() const;
};

struct TaskPromise {
    Task get_return_object() { return { Task::from_promise(*this) }; }

    std::suspend_always initial_suspend() noexcept { return {}; }
    std::suspend_always final_suspend() noexcept { return {}; }
    void return_void() { }
    void unhandled_exception() { std::abort(); }

    Task m_inner;
    Context* m_context = nullptr;

    friend struct Task;
};

void Task::await_suspend(std::coroutine_handle<TaskPromise> outer)
{
    assert(outer.promise().m_context);
    outer.promise().m_inner = *this;
    promise().m_context = outer.promise().m_context;
}

void Task::resume() const
{
    if (promise().m_inner) {
        if (!promise().m_inner.done()) {
            promise().m_inner.resume();
            return;
        }
        promise().m_inner.destroy();
        promise().m_inner = {};
    }
    std::coroutine_handle<TaskPromise>::resume();
}

class SleepAwaitable {
public:
    SleepAwaitable()
        : m_awakeTime(std::chrono::steady_clock::now())
    {
    }

    template <typename DurationT>
    SleepAwaitable(DurationT sleepFor)
        : m_awakeTime(std::chrono::steady_clock::now() + sleepFor)
    {
    }

    bool await_ready() { return false; }
    void await_suspend(std::coroutine_handle<TaskPromise>);
    void await_resume() { }

private:
    Context::Time m_awakeTime;
};

Task addTask(Task);
bool stopTask(Task&);

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
    return { std::chrono::milliseconds(time * MsPerTick / getTimeAcceleration()) };
}

void runScheduler();
void stopScheduler();

} // namespace resl
