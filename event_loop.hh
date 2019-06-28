#ifndef EVENTLOOP_HH
#define EVENTLOOP_HH

#include <atomic>
#include <functional>
#include <memory>
#include <mutex>
#include <thread>
#include <vector>

#include "callbacks.hh"
#include "timer_id.hh"
#include "timestamp.hh"

class Channel;
class EpollPoller;
class TimerQueue;

class EventLoop
{
public:
    using Functor = std::function<void()>;

    EventLoop();
    ~EventLoop();
    void Loop();
    void Quit();
    void UpdateChannel(Channel *channel);
    void RemoveChannel(Channel *channel);
    void QueueInLoop(Functor functor);
    void RunInLoop(Functor functor);
    void HandleRead();

    TimerId RunAt(Timestamp when, TimerCallback cb);
    TimerId RunAfter(double delay, TimerCallback cb);
    TimerId RunEvery(double interval, TimerCallback cb);
    void CancelTimer(TimerId timerId);

private:
    void WakeUp();
    void DoPendingFunctors();
    bool IsInLoopThread() { return std::this_thread::get_id() == threadId_; }

    std::atomic<bool> quit_;
    std::atomic<bool> callingPendingFunctors_;
    std::thread::id threadId_;
    std::unique_ptr<EpollPoller> poller_;
    std::unique_ptr<TimerQueue> timerQueue_;

    int wakeupfd_;
    std::unique_ptr<Channel> wakeupChannel_;

    std::mutex mutex_;
    std::vector<Functor> pendingFunctors_;
};

#endif