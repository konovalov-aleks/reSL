#include "task.h"

#include "scheduler.h"

namespace resl {

void SleepAwaitable::await_suspend(std::coroutine_handle<TaskPromise> hdl)
{
    hdl.promise().m_sleepUntil = m_awakeTime;
}

Task addTask(Task task)
{
    Scheduler::instance().addTask(task);
    return task;
}

bool stopTask(Task task)
{
    return Scheduler::instance().stopTask(task);
}

void runScheduler()
{
    Scheduler::instance().run();
}

void stopScheduler()
{
    Scheduler::instance().stop();
}

} // namespace resl
