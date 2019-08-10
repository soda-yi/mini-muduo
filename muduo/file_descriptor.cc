#include "file_descriptor.hh"

#include <arpa/inet.h>

void sockets::Socket::Bind(const EndPoint &endpoint) const
{
    struct sockaddr_in servaddr;
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = inet_addr(endpoint.ip_addr.c_str());
    servaddr.sin_port = htons(endpoint.port);

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
            *endpoint = EndPoint{inet_ntoa(clientAddr.sin_addr), ntohs(clientAddr.sin_port)};
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