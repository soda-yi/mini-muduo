#include "channel.hh"

#include "event_loop.hh"

#include <sys/epoll.h>

Channel::Channel(EventLoop *loop, int sockfd) noexcept
    : loop_{loop},
      fd_{sockfd}
{
}

void Channel::HandleEvent() const
{
    if (revents_ & EPOLLRDHUP) {
        if (closeCallback_) {
            closeCallback_();
        }
        return;
    }
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
}

void Channel::EnableReading() noexcept
{
    events_ |= (EPOLLIN | EPOLLRDHUP);
    Update();
}

void Channel::DisableReading() noexcept
{
    events_ &= ~EPOLLIN;
    Update();
}

void Channel::EnableWriting() noexcept
{
    events_ |= EPOLLOUT;
    Update();
}

void Channel::DisableWriting() noexcept
{
    events_ &= ~EPOLLOUT;
    Update();
}

void Channel::DisableAll() noexcept
{
    events_ = 0;
    Update();
}

bool Channel::IsWriting() const noexcept
{
    return events_ & EPOLLOUT;
}

bool Channel::IsReading() const noexcept
{
    return events_ & EPOLLIN;
}

void Channel::Add() noexcept
{
    loop_->AddChannel(*this);
    state_ = State::kAdded;
}

/* 具体实现中涉及epoll的操作，所以将实现置于poller中 */
void Channel::Update() noexcept
{
    state_ == State::kNew ? Add() : loop_->UpdateChannel(*this);
}

void Channel::Remove() noexcept
{
    loop_->RemoveChannel(*this);
}