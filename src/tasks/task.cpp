#include "task.h"

#include <cassert>
#include <chrono>
#include <list>
#include <set>
#include <thread>

namespace resl {

namespace {

    class Scheduler {
    public:
        static Scheduler& instance()
        {
            static Scheduler m_instance;
            return m_instance;
        }

        void addTask(Task&&);

        void run();
        void stop() { m_stop = true; }

    private:
        Scheduler() = default;
        ~Scheduler();

        using Tasks = std::list<Task>;

        struct AwakeTimeCompare {
        public:
            bool operator()(Tasks::iterator a, Tasks::iterator b) const noexcept
            {
                return a->promise().m_sleepUntil < b->promise().m_sleepUntil;
            }
        };

        void reset();

        Tasks m_tasks;
        std::multiset<Tasks::iterator, AwakeTimeCompare> m_waitingTasks;
        std::list<Tasks::iterator> m_readyTasks;

        bool m_stop = false;
    };

    Scheduler::~Scheduler()
    {
        reset();
    }

    void Scheduler::reset()
    {
        for (Task& task : m_tasks)
            task.destroy();
        m_tasks.clear();
        m_waitingTasks.clear();
        m_readyTasks.clear();
    }

    void Scheduler::addTask(Task&& task)
    {
        m_tasks.push_front(std::move(task));
        m_readyTasks.push_front(m_tasks.begin());
    }

    void Scheduler::run()
    {
        m_stop = false;
        while (!m_stop && !m_tasks.empty()) {
            assert(m_tasks.size() == m_waitingTasks.size() + m_readyTasks.size());
            Tasks::iterator taskToResume = m_tasks.end();

            if (auto iter = m_waitingTasks.begin();
                iter != m_waitingTasks.end() && (*iter)->promise().m_sleepUntil <= getTime()) {

                taskToResume = *iter;
                m_waitingTasks.erase(iter);
            } else if (!m_readyTasks.empty()) {
                taskToResume = m_readyTasks.front();
                m_readyTasks.erase(m_readyTasks.begin());
            }

            if (taskToResume != m_tasks.end()) {
                taskToResume->resume();
                if (taskToResume->done()) {
                    taskToResume->destroy();
                    m_tasks.erase(taskToResume);
                } else {
                    if (taskToResume->promise().m_sleepUntil == 0)
                        m_readyTasks.push_back(taskToResume);
                    else
                        m_waitingTasks.insert(taskToResume);
                }
                continue;
            }

            TimeT sleepTime = 1;
            if (!m_waitingTasks.empty()) {
                TimeT t = (*m_waitingTasks.begin())->promise().m_sleepUntil;
                TimeT now = getTime();
                if (now > t)
                    continue;
                sleepTime = t - now;
            }
            assert(m_tasks.size() == m_waitingTasks.size() + m_readyTasks.size());
            std::this_thread::sleep_for(
                std::chrono::milliseconds(MsPerTick * sleepTime));
        }
    }

} // namespace

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
