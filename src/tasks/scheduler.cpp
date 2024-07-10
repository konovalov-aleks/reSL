#include "scheduler.h"

#include "task.h"

#include <algorithm>
#include <cassert>
#include <chrono>

#ifdef __EMSCRIPTEN__
#   include <emscripten.h>
#else
#   include <thread>
#endif

namespace resl {

using Clock = std::chrono::steady_clock;

static constexpr Clock::duration s_noTasksSleepTime = std::chrono::milliseconds(10);

Scheduler::~Scheduler()
{
    reset();
}

void Scheduler::reset()
{
    for (std::pair<Task, Context>& t : m_tasks)
        t.first.destroy();
    m_tasks.clear();
    m_sleepingTasks.clear();
    m_readyTasks.clear();
}

void Scheduler::addTask(Task task)
{
    m_tasks.emplace_back(task, Context());
    task.promise().m_context = &m_tasks.back().second;
    m_readyTasks.push_front(task);
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

    auto readyIter = std::ranges::find(m_readyTasks, task);
    if (readyIter != m_readyTasks.end())
        m_readyTasks.erase(readyIter);

    // Set uses a custom comparator - it's ordered by awake time.
    // So, just 'm_sleepingTasks.erase(task)' would work incorrectly.
    auto sleepIter = std::ranges::find_if(m_sleepingTasks,
                                          [&task](Task t) { return t == task; });
    if (sleepIter != m_sleepingTasks.end())
        m_sleepingTasks.erase(sleepIter);

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
    m_readyTasks.push_front(task.promise().get_return_object());
}

void Scheduler::run()
{
    m_stop = false;
    while (!m_stop && !m_tasks.empty()) {
        validateState();

        Task taskToResume;
        if (auto iter = m_sleepingTasks.begin();
            iter != m_sleepingTasks.end() && iter->promise().m_context->m_sleepUntil <= Clock::now()) {
            taskToResume = *iter;
            m_sleepingTasks.erase(iter);
            taskToResume.promise().m_context->m_sleepUntil = std::nullopt;
        } else if (!m_readyTasks.empty()) {
            taskToResume = m_readyTasks.front();
            m_readyTasks.pop_front();
            assert(!taskToResume.promise().m_context->m_sleepUntil);
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
                if (!taskToResume.promise().m_context->m_sleepUntil)
                    m_readyTasks.push_back(taskToResume);
                else
                    m_sleepingTasks.insert(taskToResume);
            } else
                assert(!taskToResume.promise().m_context->m_sleepUntil);
            continue;
        }

        Clock::duration sleepTime = s_noTasksSleepTime;
        if (!m_sleepingTasks.empty()) {
            assert(m_sleepingTasks.begin()->promise().m_context->m_sleepUntil);
            auto t = *m_sleepingTasks.begin()->promise().m_context->m_sleepUntil;
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
    for (const auto& [task, context] : m_tasks) {
        assert(task.promise().m_context == &context);
        if (context.m_suspended) {
            // the task is waiting for a message
            assert(!context.m_sleepUntil);
            assert(m_sleepingTasks.find(task) == m_sleepingTasks.end());
            assert(std::ranges::find(m_readyTasks, task) == m_readyTasks.end());

        } else if (context.m_sleepUntil) {
            // the task is sleeping
            assert(m_sleepingTasks.find(task) != m_sleepingTasks.end());
            assert(std::ranges::find(m_readyTasks, task) == m_readyTasks.end());

        } else {
            // the task is ready for execution
            assert(m_sleepingTasks.find(task) == m_sleepingTasks.end());
            assert(std::ranges::find(m_readyTasks, task) != m_readyTasks.end());
        }
    }

    const auto existsInTasks = [this](Task task) {
        return std::ranges::find_if(m_tasks,
                                    [task](const std::pair<Task, Context>& t) {
                                        return t.first == task;
                                    }) != m_tasks.end();
    };

    for (Task t : m_readyTasks)
        assert(existsInTasks(t));
    for (Task t : m_sleepingTasks)
        assert(existsInTasks(t));

#endif // !NDEBUG
}

} // namespace resl
