#include "epoll_poller.hh"

#include <unistd.h>

#include <cerrno>

#include <iostream>

#include "channel.hh"

using std::cout;
using std::endl;

EpollPoller::EpollPoller()
    : epollfd_{::epoll_create1(EPOLL_CLOEXEC)},
      events_{kMaxEvents}
{
    if (epollfd_ < 0) {
        cout << "epoll_create error, errno:" << epollfd_ << endl;
    }
}

EpollPoller::EpollPoller(EpollPoller &&rhs)
    : epollfd_{rhs.epollfd_},
      events_{std::move(rhs.events_)}
{
    rhs.epollfd_ = -1;
}

EpollPoller &EpollPoller::operator=(EpollPoller &&rhs)
{
    if (this != &rhs) {
        epollfd_ = rhs.epollfd_;
        events_ = std::move(events_);

        rhs.epollfd_ = -1;
    }
    return *this;
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

void EpollPoller::UpdateChannel(Channel *channel) const noexcept
{
    auto state = channel->GetState();
    struct epoll_event ev;
    ev.data.ptr = channel;
    ev.events = channel->GetEvents();
    int fd = channel->GetFd();
    if (channel->GetEvents() == 0) {
        channel->SetState(Channel::State::kNew);
        ::epoll_ctl(epollfd_, EPOLL_CTL_DEL, fd, &ev);
    } else {
        if (state == Channel::State::kNew) {
            channel->SetState(Channel::State::kAdded);
            ::epoll_ctl(epollfd_, EPOLL_CTL_ADD, fd, &ev);
        } else {
            ::epoll_ctl(epollfd_, EPOLL_CTL_MOD, fd, &ev);
        }
    }
}

void EpollPoller::RemoveChannel(Channel *channel) const noexcept
{
    auto state = channel->GetState();
    struct epoll_event ev;
    ev.data.ptr = channel;
    ev.events = channel->GetEvents();
    int fd = channel->GetFd();
    if (state == Channel::State::kAdded) {
        ::epoll_ctl(epollfd_, EPOLL_CTL_DEL, fd, &ev);
    }
    channel->SetState(Channel::State::kNew);
}