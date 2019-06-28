#include "epoll_poller.hh"

#include <unistd.h>

#include <cerrno>

#include <iostream>

#include "channel.hh"

using std::cout;
using std::endl;

namespace
{
const int kNew = -1;
const int kAdded = 1;
const int kDeleted = 2;
} // namespace

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
    ::close(epollfd_);
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
    int index = channel->GetIndex();
    struct epoll_event ev;
    ev.data.ptr = channel;
    ev.events = channel->GetEvents();
    int fd = channel->GetFd();
    if (index == kNew || index == kDeleted) {
        channel->SetIndex(kAdded);
        ::epoll_ctl(epollfd_, EPOLL_CTL_ADD, fd, &ev);
    } else {
        if (channel->GetEvents() == 0) {
            channel->SetIndex(kDeleted);
            ::epoll_ctl(epollfd_, EPOLL_CTL_DEL, fd, &ev);
        } else {
            ::epoll_ctl(epollfd_, EPOLL_CTL_MOD, fd, &ev);
        }
    }
}

void EpollPoller::RemoveChannel(Channel *channel)
{
    int index = channel->GetIndex();
    struct epoll_event ev;
    ev.data.ptr = channel;
    ev.events = channel->GetEvents();
    int fd = channel->GetFd();
    if (index == kAdded) {
        ::epoll_ctl(epollfd_, EPOLL_CTL_DEL, fd, &ev);
    }
    channel->SetIndex(kNew);
}