#include "file_descriptor.hh"

#include <arpa/inet.h>

void sockets::Socket::Bind(const EndPoint &endpoint) const
{
    struct sockaddr_in servaddr;
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = endpoint.GetIpAddr();
    servaddr.sin_port = endpoint.GetPort();

    if (-1 == ::bind(GetFd(), (struct sockaddr *)&servaddr, sizeof(servaddr))) {
        throw NetException{NetExceptionKind::kBindError};
    }
}

sockets::Socket sockets::Socket::Accept(EndPoint *endpoint) const
{
    socklen_t sinLen = sizeof(sockaddr_in);
    sockaddr_in clientAddr;
    if (int newSockfd = accept(GetFd(), reinterpret_cast<sockaddr *>(&clientAddr), &sinLen); newSockfd == -1) {
        if (errno == EINTR || errno == EWOULDBLOCK || errno == EAGAIN) {
            throw NetException(NetExceptionKind::kAcceptAgain);
        }
        throw NetException(NetExceptionKind::kAcceptError);
    } else {
        if (endpoint) {
            *endpoint = EndPoint{ntohl(clientAddr.sin_addr.s_addr), ntohs(clientAddr.sin_port)};
        }
        return Socket{newSockfd};
    }
}

ssize_t FileDescriptor::Read(void *buf, std::size_t n) const
{
    if (auto sz = ::read(fd_, buf, n); sz == -1) {
        if (errno == EAGAIN || errno == EWOULDBLOCK) {
            throw FdException{FdExceptionKind::kReadAgain};
        } else if (errno == ECONNRESET || errno == ETIMEDOUT) {
            throw FdException(FdExceptionKind::kReadShutdown);
        }
        throw FdException{FdExceptionKind::kReadError};
    } else if (sz == 0) {
        throw FdException(FdExceptionKind::kReadShutdown);
    } else {
        return sz;
    }
}

ssize_t FileDescriptor::Write(const void *buf, std::size_t n) const
{
    if (auto sz = ::write(fd_, buf, n); sz == -1) {
        throw FdException{FdExceptionKind::kWriteError};
    } else {
        return sz;
    }
}

void timerfds::TimerFd::SetTime(const TimePoint &expiration) const
{
    using namespace std::chrono;
    using Clock = std::chrono::high_resolution_clock;
    using Duration = Clock::duration;

    struct itimerspec newValue;
    struct itimerspec oldValue;
    bzero(&newValue, sizeof(newValue));
    bzero(&oldValue, sizeof(oldValue));

    Duration nanoseconds = expiration.time_since_epoch() - Clock::now().time_since_epoch();
    if (nanoseconds < 100ns) {
        nanoseconds = 100ns;
    }
    auto &ts = newValue.it_value;
    ts.tv_sec = static_cast<decltype(ts.tv_sec)>(duration_cast<seconds>(nanoseconds).count());
    ts.tv_nsec = static_cast<decltype(ts.tv_nsec)>(nanoseconds.count() % 1000000000);

    ::timerfd_settime(GetFd(), 0, &newValue, &oldValue);
}

void timerfds::TimerFd::CancelTime() const noexcept
{
    struct itimerspec newValue;
    bzero(&newValue, sizeof(newValue));
    ::timerfd_settime(GetFd(), 0, &newValue, nullptr);
}