#ifndef EVENT_LOOP_THREAD_HH
#define EVENT_LOOP_THREAD_HH

#include <condition_variable>
#include <memory>
#include <mutex>
#include <thread>

#include "event_loop.hh"

/**
 * @brief EventLoopThread
 * 
 * 为one loop per thread创建的类，使用此类来管理loop的生存期
 */
class EventLoopThread
{
public:
    EventLoopThread() = default;
    EventLoopThread(const EventLoopThread &) = delete;
    EventLoopThread &operator=(const EventLoopThread &) = delete;
    EventLoopThread(EventLoopThread &&) = default;
    EventLoopThread &operator=(EventLoopThread &&) = default;
    ~EventLoopThread();

    /**
     * @brief EventLoop工厂方法，创建并启动一个EventLoop
     * 
     * @return EventLoop* 创建的EventLoop
     */
    EventLoop *StartLoop();

private:
    /**
     * @brief 新线程中执行的操作
     * 
     * 在新线程中执行EventLoop::Loop操作
     */
    void ThreadFunc();

    EventLoop *loop_{nullptr};
    bool exiting_{false};
    std::unique_ptr<std::thread> thread_{nullptr};
    std::mutex mutex_;
    std::condition_variable cond_;
};

#endif