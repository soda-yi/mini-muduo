#include "event_loop.hh"

#include <sys/eventfd.h>
#include <unistd.h>

#include <iostream>
#include <vector>

#include "channel.hh"
#include "epoll_poller.hh"

using std::cout;
using std::endl;

EventLoop::EventLoop()
    : quit_(false),
      poller_(new EpollPoller()),
      wakeupfd_(::eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC)),
      wakeupChannel_(new Channel(this, wakeupfd_))
{
    if (wakeupfd_ < 0) {
        cout << "Failed in eventfd" << endl;
        abort();
    }
    wakeupChannel_->SetReadCallback(std::bind(&EventLoop::HandleRead, this));
    wakeupChannel_->EnableReading();
}

EventLoop::~EventLoop()
{
}

void EventLoop::Loop()
{
    while (!quit_) {
        std::vector<Channel *> activeChannels;
        poller_->Poll(&activeChannels);
        for (auto &channel : activeChannels) {
            channel->HandleEvent();
        }
        DoPendingFunctors();
    }
}

void EventLoop::UpdateChannel(Channel *channel)
{
    poller_->UpdateChannel(channel);
}

void EventLoop::QueueLoop(Functor functor)
{
    pendingFunctors_.push_back(functor);
    WakeUp();
}

void EventLoop::WakeUp()
{
    uint64_t one = 1;
    ssize_t n = ::write(wakeupfd_, &one, sizeof(one));
    if (n != sizeof(one)) {
        cout << "EventLoop::wakeup() writes " << n << " bytes instead of 8" << endl;
    }
}

void EventLoop::HandleRead()
{
    uint64_t one = 1;
    ssize_t n = ::read(wakeupfd_, &one, sizeof(one));
    if (n != sizeof(one)) {
        cout << "EventLoop::handleRead() reads " << n << " bytes instead of 8" << endl;
    }
}

void EventLoop::DoPendingFunctors()
{
    std::vector<Functor> functors;
    functors.swap(pendingFunctors_);

    for (auto &functor : functors) {
        functor();
    }
}