#include "channel.hh"

#include <sys/epoll.h>

Channel::Channel(int epollfd, int sockfd)
    : epollfd_(epollfd),
      sockfd_(sockfd)
{
}

Channel::~Channel()
{
}

void Channel::HandleEvent()
{
    if (revents_ & EPOLLIN) {
        callback_(sockfd_);
    }
}

void Channel::EnableReading()
{
    events_ |= EPOLLIN;
    Update();
}

void Channel::Update()
{
    struct epoll_event ev;
    ev.data.ptr = this;
    ev.events = events_;
    epoll_ctl(epollfd_, EPOLL_CTL_ADD, sockfd_, &ev);
}