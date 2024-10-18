#pragma once
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable> 

// 异步写日志的日志队列
template <typename T>
class LockQueue {
public:
    void push(const T &data);
    T pop();
    bool isEmpty() const;
private:
    std::queue<T> m_queue;
    std::mutex m_mutex;
    std::condition_variable m_condVar;
};

template <typename T>
inline void LockQueue<T>::push(const T &data)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    m_queue.push(data);
    m_condVar.notify_one(); // 只有一个写日志线程
}

template <typename T>
inline T LockQueue<T>::pop()
{
    std::unique_lock<std::mutex> lock(m_mutex);
    while (m_queue.empty()) {
        // 日志队列为空，线程进入wait状态
        m_condVar.wait(lock);
    }

    T data = m_queue.front();
    m_queue.pop();
    return data;
}

template <typename T>
inline bool LockQueue<T>::isEmpty() const
{
    return m_queue.empty();
}
