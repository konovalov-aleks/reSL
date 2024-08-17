#pragma once

#include "task.h"

#include <deque>
#include <list>
#include <utility>
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

    Task chooseTaskToResume();

    void reset();

    void validateState();

    std::list<std::pair<Task, Context>> m_tasks;

    // Tasks ready to be executed starting at a certain time.
    // Binary heap, the key is "m_sleepUntil"
    std::vector<Task> m_queue;

    bool m_stop = false;
};

} // namespace resl
