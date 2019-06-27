#include "epoll_poller.hh"

#include <cerrno>

#include <iostream>

#include "channel.hh"

using std::cout;
using std::endl;

EpollPoller::EpollPoller()
    : epollfd_(::epoll_create1(EPOLL_CLOEXEC)),
      events_(kMaxEvents)
{
    if (epollfd_ < 0) {
        cout << "epoll_create error, errno:" << epollfd_ << endl;
    }
}

EpollPoller::~EpollPoller()
{
}

void EpollPoller::Poll(ChannelList *activeChannels)
{
    int numEvents = ::epoll_wait(epollfd_,
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

void EpollPoller::UpdateChannel(Channel *channel)
{
    struct epoll_event ev;
    ev.data.ptr = channel;
    ev.events = channel->GetEvents();
    int fd = channel->GetSockfd();
    ::epoll_ctl(epollfd_, EPOLL_CTL_ADD, fd, &ev);
}