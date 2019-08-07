#ifndef EVENTLOOP_HH
#define EVENTLOOP_HH

#include <atomic>
#include <functional>
#include <memory>
#include <mutex>
#include <thread>
#include <vector>

#include "callbacks.hh"
#include "channel.hh"
#include "epoll_poller.hh"
#include "timer_id.hh"
#include "timestamp.hh"

class EpollPoller;
class TimerQueue;

class EventLoop
{
public:
    using Functor = std::function<void()>;

    EventLoop();
    EventLoop(const EventLoop &) = delete;
    EventLoop &operator=(const EventLoop &) = delete;
    // FIXME 移动构造不能是默认的
    EventLoop(EventLoop &&) = default;
    EventLoop &operator=(EventLoop &&) = default;
    ~EventLoop();

    void Loop();
    void Quit() noexcept;
    void UpdateChannel(Channel *channel) const noexcept;
    void RemoveChannel(Channel *channel) const noexcept;
    void QueueInLoop(Functor functor);
    void RunInLoop(Functor functor);
    void HandleRead() const noexcept;

    [[deprecated]] TimerId RunAt(Timestamp when, TimerCallback cb);
    [[deprecated]] TimerId RunAfter(double delay, TimerCallback cb);
    [[deprecated]] TimerId RunEvery(double interval, TimerCallback cb);
    [[deprecated]] void CancelTimer(TimerId timerId);

private:
    void WakeUp() const noexcept;
    void DoPendingFunctors();
    bool IsInLoopThread() const noexcept { return std::this_thread::get_id() == threadId_; }

    std::atomic<bool> quit_;
    std::atomic<bool> callingPendingFunctors_;
    std::thread::id threadId_;
    EpollPoller poller_;
    std::unique_ptr<TimerQueue> timerQueue_;

    int wakeupfd_;
    Channel wakeupChannel_;

    std::mutex mutex_;
    std::vector<Functor> pendingFunctors_;
};

#endif