#include "task.h"

#include "scheduler.h"

#include <cassert>

namespace resl {

void SleepAwaitable::await_suspend(std::coroutine_handle<TaskPromise> hdl)
{
    assert(hdl.promise().m_context);
    hdl.promise().m_context->m_sleepUntil = m_awakeTime;
}

Task addTask(Task task)
{
    Scheduler::instance().addTask(task);
    return task;
}

bool stopTask(Task& task)
{
    bool res = Scheduler::instance().stopTask(task);
    task = {};
    return res;
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
