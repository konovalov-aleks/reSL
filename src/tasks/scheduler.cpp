#include "scheduler.h"

#include "task.h"

#include <algorithm>
#include <cassert>
#include <chrono>
#include <thread>

namespace resl {

using Clock = std::chrono::steady_clock;

static constexpr Clock::duration s_noTasksSleepTime = std::chrono::milliseconds(10);

Scheduler::~Scheduler()
{
    reset();
}

void Scheduler::reset()
{
    for (Task& task : m_tasks)
        task.destroy();
    m_tasks.clear();
    m_sleepingTasks.clear();
    m_readyTasks.clear();
}

void Scheduler::addTask(Task task)
{
    m_tasks.push_back(task);
    if (!task.promise().m_suspended)
        m_readyTasks.push_front(task);
}

void Scheduler::resumeTask(std::coroutine_handle<TaskPromise> task)
{
    if (!task.promise().m_suspended)
        return;

    // the task must be located in m_tasks
    assert(std::find_if(m_tasks.begin(), m_tasks.end(),
                        [&task](const Task& t) { return &t.promise() == &task.promise(); }) != m_tasks.end());
    task.promise().m_suspended = false;
    m_readyTasks.push_front(task.promise().get_return_object());
}

void Scheduler::run()
{
    m_stop = false;
    while (!m_stop && !m_tasks.empty()) {
        assert(m_tasks.size() == m_sleepingTasks.size() + m_readyTasks.size() + std::count_if(m_tasks.begin(), m_tasks.end(), [](const Task& t) { return t.promise().m_suspended; }));

        Task taskToResume;
        if (auto iter = m_sleepingTasks.begin();
            iter != m_sleepingTasks.end() && iter->promise().m_sleepUntil <= Clock::now()) {
            taskToResume = *iter;
            m_sleepingTasks.erase(iter);
        } else if (!m_readyTasks.empty()) {
            taskToResume = m_readyTasks.front();
            m_readyTasks.pop_front();
        }

        if (taskToResume) {
            assert(!taskToResume.promise().m_suspended);
            taskToResume.resume();
            if (taskToResume.done()) {
                auto iter = std::find(m_tasks.begin(), m_tasks.end(), taskToResume);
                assert(iter != m_tasks.end());
                taskToResume.destroy();
                m_tasks.erase(iter);
            } else if (!taskToResume.promise().m_suspended) {
                if (!taskToResume.promise().m_sleepUntil)
                    m_readyTasks.push_back(taskToResume);
                else
                    m_sleepingTasks.insert(taskToResume);
            }
            continue;
        }

        Clock::duration sleepTime = s_noTasksSleepTime;
        if (!m_sleepingTasks.empty()) {
            assert(m_sleepingTasks.begin()->promise().m_sleepUntil);
            auto t = *m_sleepingTasks.begin()->promise().m_sleepUntil;
            const Clock::time_point now = Clock::now();
            if (now > t)
                continue;
            sleepTime = t - now;
        }
        assert(m_tasks.size() == m_sleepingTasks.size() + m_readyTasks.size() + std::count_if(m_tasks.begin(), m_tasks.end(), [](const Task& t) { return t.promise().m_suspended; }));
        std::this_thread::sleep_for(sleepTime);
    }
}

} // namespace resl
