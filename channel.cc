#include "channel.hh"

#include "event_loop.hh"

#include <sys/epoll.h>

Channel::Channel(EventLoop *loop, int sockfd)
    : loop_(loop),
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
    loop_->UpdateChannel(this);
}