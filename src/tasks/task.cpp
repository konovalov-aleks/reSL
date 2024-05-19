#include "task.h"

#include "scheduler.h"

namespace resl {

void SleepAwaitable::await_suspend(std::coroutine_handle<TaskPromise> hdl)
{
    hdl.promise().m_sleepUntil = m_awakeTime;
}

void addTask(Task&& task)
{
    Scheduler::instance().addTask(std::move(task));
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
