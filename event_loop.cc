#include "event_loop.hh"

#include <vector>

#include "channel.hh"
#include "epoll_poller.hh"

EventLoop::EventLoop()
    : quit_(false), poller_(new EpollPoller()) // Memory Leak !!!
{
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
    }
}

void EventLoop::UpdateChannel(Channel *channel)
{
    poller_->UpdateChannel(channel);
}