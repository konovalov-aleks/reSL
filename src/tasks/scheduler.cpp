#include "scheduler.h"

#include "task.h"

#include <algorithm>
#include <cassert>
#include <chrono>
#include <compare>

#ifdef __EMSCRIPTEN__
#   include <emscripten.h>
#else
#   include <thread>
#endif

namespace resl {

namespace {

    struct AwakeTimeCompare {
    public:
        bool operator()(const Task& a, const Task& b) const noexcept
        {
            assert(a.promise().m_context);
            assert(b.promise().m_context);
            return a.promise().m_context->m_sleepUntil > b.promise().m_context->m_sleepUntil;
        }
    };

    using Clock = std::chrono::steady_clock;

    static constexpr Clock::duration s_idleSleepTime = std::chrono::milliseconds(10);

} // namepsace

Scheduler::~Scheduler()
{
    reset();
}

void Scheduler::reset()
{
    for (std::pair<Task, Context>& t : m_tasks)
        t.first.destroy();
    m_tasks.clear();
    m_queue.clear();
}

void Scheduler::addTask(Task task)
{
    m_tasks.emplace_back(task, Context());
    task.promise().m_context = &m_tasks.back().second;

    m_queue.push_back(task);
    std::ranges::push_heap(m_queue, AwakeTimeCompare());
}

bool Scheduler::stopTask(Task task)
{
    auto iter = std::ranges::find_if(m_tasks,
                                     [task](const std::pair<Task, Context>& t) {
                                         return task == t.first;
                                     });
    if (iter == m_tasks.end())
        return false;
    m_tasks.erase(iter);

    auto queueIter = std::ranges::find(m_queue, task);
    if (queueIter != m_queue.end()) {
        m_queue.erase(queueIter);
        std::ranges::make_heap(m_queue, AwakeTimeCompare());
    }

    task.destroy();
    return true;
}

void Scheduler::resumeTask(std::coroutine_handle<TaskPromise> task)
{
    assert(task.promise().m_context);
    if (!task.promise().m_context->m_suspended)
        return;

    // the task must be located in m_tasks
    assert(std::find_if(m_tasks.begin(), m_tasks.end(),
                        [&task](const std::pair<Task, Context>& t) {
                            return &t.first.promise() == &task.promise();
                        }) != m_tasks.end());
    task.promise().m_context->m_suspended = false;
    task.promise().m_context->m_sleepUntil = Clock::now();

    m_queue.push_back(task.promise().get_return_object());
    std::ranges::push_heap(m_queue, AwakeTimeCompare());
}

void Scheduler::run()
{
    m_stop = false;
    while (!m_stop && !m_tasks.empty()) {
        validateState();

        Task taskToResume;
        if (!m_queue.empty() && m_queue.front().promise().m_context->m_sleepUntil <= Clock::now()) {
            taskToResume = m_queue.front();
            std::ranges::pop_heap(m_queue, AwakeTimeCompare());
            m_queue.pop_back();
        }

        if (taskToResume) {
            assert(!taskToResume.promise().m_context->m_suspended);
            taskToResume.resume();
            if (taskToResume.done()) {
                auto iter = std::find_if(m_tasks.begin(), m_tasks.end(),
                                         [taskToResume](const std::pair<Task, Context>& t) {
                                             return t.first == taskToResume;
                                         });
                assert(iter != m_tasks.end());
                taskToResume.destroy();
                m_tasks.erase(iter);
            } else if (!taskToResume.promise().m_context->m_suspended) {
                m_queue.push_back(taskToResume);
                std::ranges::push_heap(m_queue, AwakeTimeCompare());
            }
            continue;
        }

        Clock::duration sleepTime = s_idleSleepTime;
        if (!m_queue.empty()) {
            auto t = m_queue.front().promise().m_context->m_sleepUntil;
            const Clock::time_point now = Clock::now();
            if (now > t)
                continue;
            sleepTime = t - now;
        }

#ifdef __EMSCRIPTEN__
        emscripten_sleep(std::chrono::duration_cast<std::chrono::milliseconds>(sleepTime).count());
#else
        std::this_thread::sleep_for(sleepTime);
#endif // __EMSCRIPTEN__
    }
}

void Scheduler::validateState()
{
#ifndef NDEBUG
    assert(std::ranges::is_heap(m_queue, AwakeTimeCompare()));
    for (const auto& [task, context] : m_tasks) {
        assert(task.promise().m_context == &context);
        if (context.m_suspended) {
            // the task is waiting for a message
            assert(std::ranges::find(m_queue, task) == m_queue.end());

        } else {
            // sleeping or active task
            assert(std::ranges::find(m_queue, task) != m_queue.end());
        }
    }

    for (Task task : m_queue) {
        assert(std::ranges::find_if(m_tasks,
                                    [task](const std::pair<Task, Context>& t) {
                                        return t.first == task;
                                    }) != m_tasks.end());
    }
#endif // !NDEBUG
}

} // namespace resl
