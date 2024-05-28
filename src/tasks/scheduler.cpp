#include "scheduler.h"

#include "task.h"
#include <system/time.h>

#include <algorithm>
#include <cassert>
#include <chrono>
#include <thread>

namespace resl {

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
            iter != m_sleepingTasks.end() && iter->promise().m_sleepUntil <= getTime()) {
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
                if (taskToResume.promise().m_sleepUntil == 0)
                    m_readyTasks.push_front(taskToResume);
                else
                    m_sleepingTasks.insert(taskToResume);
            }
            continue;
        }

        TimeT sleepTime = 1;
        if (!m_sleepingTasks.empty()) {
            TimeT t = m_sleepingTasks.begin()->promise().m_sleepUntil;
            TimeT now = getTime();
            if (now > t)
                continue;
            sleepTime = t - now;
        }
        assert(m_tasks.size() == m_sleepingTasks.size() + m_readyTasks.size() + std::count_if(m_tasks.begin(), m_tasks.end(), [](const Task& t) { return t.promise().m_suspended; }));
        std::this_thread::sleep_for(
            std::chrono::milliseconds(MsPerTick * sleepTime));
    }
}

} // namespace resl
