#pragma once

#include "task.h"

#include <chrono>
#include <compare>
#include <deque>
#include <optional>
#include <set>
#include <vector>

namespace resl {

class Scheduler {
public:
    static Scheduler& instance()
    {
        static Scheduler m_instance;
        return m_instance;
    }

    void addTask(Task);
    bool stopTask(Task);
    void resumeTask(std::coroutine_handle<TaskPromise>);

    void run();
    void stop() { m_stop = true; }

private:
    Scheduler() = default;
    ~Scheduler();

    using Tasks = std::deque<Task>;

    struct AwakeTimeCompare {
    public:
        bool operator()(const Task& a, const Task& b) const noexcept
        {
            return a.promise().m_sleepUntil < b.promise().m_sleepUntil;
        }
    };

    void reset();

    std::vector<Task> m_tasks;

    std::deque<Task> m_readyTasks;
    std::multiset<Task, AwakeTimeCompare> m_sleepingTasks;

    bool m_stop = false;
};

} // namespace resl
