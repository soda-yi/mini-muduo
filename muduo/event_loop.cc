#include "event_loop.hh"

#include <sys/eventfd.h>
#include <unistd.h>

#include <iostream>
#include <vector>

#include "timer_queue.hh"

using std::cout;
using std::endl;

EventLoop::EventLoop()
    : quit_{false},
      callingPendingFunctors_{false},
      threadId_{std::this_thread::get_id()},
      poller_{},
      timerQueue_(std::make_unique<TimerQueue>(this)),
      wakeupfd_{::eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC)},
      wakeupChannel_{this, wakeupfd_.GetFd()}
{
    wakeupChannel_.SetReadCallback([this] { HandleRead(); });
    wakeupChannel_.Add();
    wakeupChannel_.EnableReading();
}

EventLoop::~EventLoop()
{
    wakeupChannel_.DisableAll();
    wakeupChannel_.Remove();
}

void EventLoop::Loop()
{
    while (!quit_) {
        std::vector<Channel *> activeChannels;
        poller_.Poll(&activeChannels);
        for (auto &channel : activeChannels) {
            channel->HandleEvent();
        }
        DoPendingFunctors();
    }
}

void EventLoop::Quit() noexcept
{
    quit_ = true;
    if (!IsInLoopThread()) {
        WakeUp();
    }
}

void EventLoop::RunInLoop(Functor functor)
{
    if (IsInLoopThread()) {
        functor();
    } else {
        QueueInLoop(std::move(functor));
    }
}

void EventLoop::QueueInLoop(Functor functor)
{
    {
        std::scoped_lock<std::mutex> lock(mutex_);
        pendingFunctors_.push_back(functor);
    }
    if (!IsInLoopThread() || callingPendingFunctors_) {
        WakeUp();
    }
}

void EventLoop::WakeUp() const
{
    uint64_t one = 1;
    ssize_t n = wakeupfd_.Write(&one, sizeof(one));
    if (n != sizeof(one)) {
        cout << "EventLoop::wakeup() writes " << n << " bytes instead of 8" << endl;
    }
}

void EventLoop::HandleRead() const
{
    uint64_t one = 1;
    ssize_t n = wakeupfd_.Read(&one, sizeof(one));
    if (n != sizeof(one)) {
        cout << "EventLoop::handleRead() reads " << n << " bytes instead of 8" << endl;
    }
}

TimerId EventLoop::RunAt(Timestamp when, TimerCallback cb)
{
    return timerQueue_->AddTimer(cb, when, 0.0);
}

TimerId EventLoop::RunAfter(double delay, TimerCallback cb)
{
    Timestamp time(AddTime(Timestamp::Now(), delay));
    return RunAt(time, std::move(cb));
}

TimerId EventLoop::RunEvery(double interval, TimerCallback cb)
{
    Timestamp time(AddTime(Timestamp::Now(), interval));
    return timerQueue_->AddTimer(std::move(cb), time, interval);
}

void EventLoop::CancelTimer(TimerId timerId)
{
    timerQueue_->CancelTimer(timerId);
}

void EventLoop::DoPendingFunctors()
{
    std::vector<Functor> functors;
    callingPendingFunctors_ = true;

    {
        std::scoped_lock<std::mutex> lock(mutex_);
        functors.swap(pendingFunctors_);
    }

    for (auto &functor : functors) {
        functor();
    }

    callingPendingFunctors_ = false;
}