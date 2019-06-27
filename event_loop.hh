#ifndef EVENTLOOP_HH
#define EVENTLOOP_HH

#include <functional>
#include <memory>
#include <vector>

#include "callback.hh"
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
    void UpdateChannel(Channel *channel);
    void QueueLoop(Functor functor);
    void HandleRead();

    TimerId RunAt(Timestamp when, TimerCallback cb);
    TimerId RunAfter(double delay, TimerCallback cb);
    TimerId RunEvery(double interval, TimerCallback cb);
    void CancelTimer(TimerId timerId);

private:
    void WakeUp();
    void DoPendingFunctors();

    bool quit_;
    std::unique_ptr<EpollPoller> poller_;
    std::unique_ptr<TimerQueue> timerQueue_;
    int wakeupfd_;
    std::unique_ptr<Channel> wakeupChannel_;
    std::vector<Functor> pendingFunctors_;
};

#endif