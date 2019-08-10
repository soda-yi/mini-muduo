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
#include "file_descriptor.hh"
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
    EventLoop(EventLoop &&) noexcept;
    EventLoop &operator=(EventLoop &&) noexcept;
    ~EventLoop();

    void Loop();
    void Quit() noexcept;
    void AddChannel(const Channel &channel) const { poller_.AddChannel(channel); }
    void UpdateChannel(const Channel &channel) const { poller_.UpdateChannel(channel); }
    void RemoveChannel(const Channel &channel) const { poller_.RemoveChannel(channel); }
    void QueueInLoop(Functor functor);
    void RunInLoop(Functor functor);
    void HandleRead() const;

    [[deprecated]] TimerId RunAt(Timestamp when, TimerCallback cb);
    [[deprecated]] TimerId RunAfter(double delay, TimerCallback cb);
    [[deprecated]] TimerId RunEvery(double interval, TimerCallback cb);
    [[deprecated]] void CancelTimer(TimerId timerId);

private:
    void WakeUp() const;
    void DoPendingFunctors();
    bool IsInLoopThread() const noexcept { return std::this_thread::get_id() == threadId_; }

    std::atomic<bool> quit_;
    std::atomic<bool> callingPendingFunctors_;
    std::thread::id threadId_;
    EpollPoller poller_;
    std::unique_ptr<TimerQueue> timerQueue_;

    FileDescriptor wakeupfd_;
    Channel wakeupChannel_;

    std::mutex mutex_;
    std::vector<Functor> pendingFunctors_;
};

#endif