#ifndef EXCEPTION_HH
#define EXCEPTION_HH

#include <cerrno>
#include <cstring>
#include <stdexcept>
#include <unistd.h>

class BaseException : public std::runtime_error
{
public:
    BaseException()
        : std::runtime_error{strerror(errno)},
          errno_{errno},
          pid_{getpid()}
    {
    }

    int GetErrno() const noexcept { return errno_; }
    pid_t GetPid() const noexcept { return pid_; }

private:
    const int errno_;
    const pid_t pid_;
};

enum class FdExceptionKind {
    kReadError,
    kReadAgain,
    kReadShutdown,
    kWriteError,
    kWriteAgain,
};

class FdException : public BaseException
{
public:
    FdException(FdExceptionKind kind)
        : exceptionKind_{kind}
    {
    }

private:
    FdExceptionKind exceptionKind_;
};

enum class NetExceptionKind {
    kBindError,
    kListenError,
    kEpollError,
    kEpollTimeout,
    kAcceptError,
    kAcceptAgain,
    kUnknownException
};

class NetException : public BaseException
{
public:
    NetException(NetExceptionKind exceptionKind)
        : BaseException{},
          exceptionKind_{exceptionKind}
    {
    }

private:
    const NetExceptionKind exceptionKind_ = NetExceptionKind::kUnknownException;
};

#endif