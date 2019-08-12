#ifndef EVENT_LOOP_THREAD_POOL_HH
#define EVENT_LOOP_THREAD_POOL_HH

#include <functional>
#include <memory>
#include <vector>

#include "event_loop_thread.hh"

/**
 * @brief EventLoopThreadPool类
 * 
 * EventLoopThread的线程池
 */
class EventLoopThreadPool
{
public:
    /**
     * @brief 构造一个EventLoopThreadPool
     * 
     * @param baseLoop 主EventLoop
     * @param numThread 线程数量，也即需要创建的除主EventLoop数量
     */
    EventLoopThreadPool(EventLoop *baseLoop, int numThread);
    EventLoopThreadPool(const EventLoopThreadPool &) = delete;
    EventLoopThreadPool &operator=(const EventLoopThreadPool &) = delete;
    EventLoopThreadPool(EventLoopThreadPool &&) = default;
    EventLoopThreadPool &operator=(EventLoopThreadPool &&) = default;

    /**
     * @brief 获取一个EventLoop
     * 
     * @return EventLoop* 一个可用的EventLoop。当线程池创建的线程数为0时，返回主EventLoop，否则在EventLoop集合中Next获取可用的EventLoop
     * 
     * 从环状EventLoop集合中Next获取可用的EventLoop
     */
    EventLoop *GetNextLoop();

private:
    EventLoop *baseLoop_;
    bool started_;
    int next_;
    std::vector<std::unique_ptr<EventLoopThread>> threads_;
    std::vector<EventLoop *> loops_;
};

#endif