#include "epoll_poller.hh"

#include <cerrno>

#include <iostream>

#include "channel.hh"

using std::cout;
using std::endl;

EpollPoller::EpollPoller() noexcept
    : epollfd_{::epoll_create1(EPOLL_CLOEXEC)},
      events_{kMaxEvents}
{
}

void EpollPoller::Poll(ChannelList *activeChannels)
{
    int numEvents = ::epoll_wait(epollfd_.GetFd(),
                                 &*events_.begin(),
                                 static_cast<int>(events_.size()), -1);
    if (numEvents == -1) {
        cout << "epoll_wait error, errno:" << errno << endl;
        return;
    }
    for (int i = 0; i < numEvents; ++i) {
        Channel *pChannel = static_cast<Channel *>(events_[i].data.ptr);
        pChannel->SetRevents(events_[i].events);
        activeChannels->push_back(pChannel);
    }
}

void EpollPoller::AddChannel(const Channel &channel) const noexcept
{
    struct epoll_event ev;
    ev.data.ptr = const_cast<Channel *>(&channel);
    ev.events = channel.GetEvents();
    int fd = channel.GetFd();
    ::epoll_ctl(epollfd_.GetFd(), EPOLL_CTL_ADD, fd, &ev);
}

void EpollPoller::UpdateChannel(const Channel &channel) const noexcept
{
    struct epoll_event ev;
    ev.data.ptr = const_cast<Channel *>(&channel);
    ev.events = channel.GetEvents();
    int fd = channel.GetFd();
    ::epoll_ctl(epollfd_.GetFd(), EPOLL_CTL_MOD, fd, &ev);
}

void EpollPoller::RemoveChannel(const Channel &channel) const noexcept
{
    struct epoll_event ev;
    ev.data.ptr = const_cast<Channel *>(&channel);
    ev.events = channel.GetEvents();
    int fd = channel.GetFd();
    ::epoll_ctl(epollfd_.GetFd(), EPOLL_CTL_DEL, fd, &ev);
}