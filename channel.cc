#include "channel.hh"

#include "event_loop.hh"

#include <sys/epoll.h>

Channel::Channel(EventLoop *loop, int sockfd)
    : loop_(loop),
      fd_(sockfd)
{
}

Channel::~Channel()
{
}

void Channel::HandleEvent()
{
    if (revents_ & EPOLLIN) {
        if (readCallback_) {
            readCallback_();
        }
    }
    if (revents_ & EPOLLOUT) {
        if (writeCallback_) {
            writeCallback_();
        }
    }
    if ((revents_ & EPOLLHUP) && !(revents_ & EPOLLIN)) {
        if (closeCallback_) {
            closeCallback_();
        }
    }
}

void Channel::EnableReading()
{
    events_ |= EPOLLIN;
    Update();
}

void Channel::DisableReading()
{
    events_ &= ~EPOLLIN;
    Update();
}

void Channel::EnableWriting()
{
    events_ |= EPOLLOUT;
    Update();
}

void Channel::DisableWriting()
{
    events_ &= ~EPOLLOUT;
    Update();
}

void Channel::DisableAll()
{
    events_ = 0;
    Update();
}

bool Channel::IsWriting() const
{
    return events_ & EPOLLOUT;
}

bool Channel::IsReading() const
{
    return events_ & EPOLLIN;
}

/* 具体实现中涉及epoll的操作，所以将实现置于poller中 */
void Channel::Update()
{
    loop_->UpdateChannel(this);
}

void Channel::Remove()
{
    loop_->RemoveChannel(this);
}