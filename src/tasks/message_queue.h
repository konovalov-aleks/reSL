#pragma once

#include "scheduler.h"
#include "task.h"

#include <coroutine>
#include <cstdlib>
#include <queue>
#include <utility>

namespace resl {

template <typename MsgT>
class MessageQueue {
public:
    class Awaitable {
    public:
        Awaitable(MessageQueue& q)
            : m_msgQueue(&q)
        {
        }

        bool await_ready() { return !m_msgQueue->m_queue.empty(); }
        void await_suspend(std::coroutine_handle<TaskPromise> task)
        {
            task.promise().m_suspended = true;
            m_msgQueue->m_waitingTask = task;
        }

        MsgT await_resume()
        {
            assert(!m_msgQueue->m_queue.empty());
            MsgT result = std::move(m_msgQueue->m_queue.front());
            m_msgQueue->m_queue.pop();
            return result;
        }

    private:
        MessageQueue* m_msgQueue;
    };

    Awaitable pop()
    {
        return { *this };
    }

    void push(MsgT msg)
    {
        m_queue.push(std::move(msg));
        if (m_waitingTask)
            Scheduler::instance().resumeTask(m_waitingTask);
    }

    void clear()
    {
        while (!m_queue.empty())
            m_queue.pop();
    }

private:
    std::queue<MsgT> m_queue;
    std::coroutine_handle<TaskPromise> m_waitingTask;
};

} // namespace resl
