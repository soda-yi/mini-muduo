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
      timerQueue_{this},
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

void EventLoop::Quit()
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
        std::scoped_lock lock{mutex_};
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

Timer::Id EventLoop::RunAt(Timer::TimePoint when, TimerCallback doWhat)
{
    using namespace std::chrono;
    return timerQueue_.AddTimer(when, 0s, std::move(doWhat));
}

Timer::Id EventLoop::RunAfter(Timer::Duration delay, TimerCallback doWhat)
{
    auto time{Timer::Clock::now() + delay};
    return RunAt(time, std::move(doWhat));
}

Timer::Id EventLoop::RunEvery(Timer::Duration interval, TimerCallback doWhat)
{
    auto time{Timer::Clock::now() + interval};
    return timerQueue_.AddTimer(time, interval, std::move(doWhat));
}

void EventLoop::CancelTimer(Timer::Id timerId)
{
    timerQueue_.CancelTimer(timerId);
}

void EventLoop::DoPendingFunctors()
{
    std::vector<Functor> functors;
    callingPendingFunctors_ = true;

    {
        std::scoped_lock lock{mutex_};
        functors.swap(pendingFunctors_);
    }

    for (auto &functor : functors) {
        functor();
    }

    callingPendingFunctors_ = false;
}