#ifndef FILE_DESCRIPTOR_HH
#define FILE_DESCRIPTOR_HH

#include <fcntl.h>
#include <sys/eventfd.h>
#include <sys/socket.h>
#include <type_traits>
#include <unistd.h>

#include <memory>

#include "exception.hh"
#include "network.hh"

class FileDescriptor
{
private:
    static constexpr int kInvalidFd = -1;

public:
    explicit FileDescriptor(int fd) noexcept
        : fd_{fd}
    {
    }
    FileDescriptor(const FileDescriptor &) = delete;
    FileDescriptor &operator=(const FileDescriptor &) = delete;
    FileDescriptor(FileDescriptor &&rhs) noexcept
        : fd_{rhs.fd_}
    {
        rhs.fd_ = kInvalidFd;
    }
    FileDescriptor &operator=(FileDescriptor &&rhs) noexcept
    {
        if (&rhs != this) {
            fd_ = rhs.fd_;
            rhs.fd_ = -1;
        }
        return *this;
    }
    /**
     * @brief 析构一个文件描述符
     * @warning 非virtual的析构函数，意味着继承其的类最好不要定义自己的析构函数
     * 
     * 关闭一个文件描述符
     */
    ~FileDescriptor()
    {
        fd_ != kInvalidFd ? Close() : void();
    }

    /**
     * @brief 关闭一个文件描述符
     */
    void Close() noexcept
    {
        ::close(fd_);
        fd_ = kInvalidFd;
    }

    /**
     * @brief 获取一个文件描述符
     */
    int GetFd() const noexcept { return fd_; }

    /**
     * @brief 往fd对应的文件中写数据
     * 
     * @param buf 
     * @param n 需要写的字节数
     * @return ssize_t 本次写成功的字节数
     */
    virtual ssize_t Write(const void *buf, std::size_t n) const;

    /**
     * @brief 从fd对应的文件中读数据
     * 
     * @param buf 
     * @param n 需要读的字节数
     * @return ssize_t 本次读到的字节数
     */
    virtual ssize_t Read(void *buf, std::size_t n) const;

    void SetNonBlock() noexcept
    {
        fcntl(fd_, F_SETFL, O_NONBLOCK);
    }

private:
    int fd_;
};

namespace sockets
{

class Socket : public FileDescriptor
{
public:
    using FileDescriptor::FileDescriptor;

    Socket(const Socket &) = delete;
    Socket &operator=(const Socket &) = delete;
    Socket(Socket &&) = default;
    Socket &operator=(Socket &&) = default;

    void SetReuseAddr() const
    {
        int on = 1;
        setsockopt(GetFd(), SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));
    }
    void Bind(const EndPoint &endPoint) const;
    void Listen() const
    {
        constexpr int backlog = 10;
        if (-1 == listen(GetFd(), backlog)) {
            throw NetException{NetExceptionKind::kListenError};
        }
    }
    Socket Accept(EndPoint *endpoint) const;
    void ShutdownWrite() const { ::shutdown(GetFd(), SHUT_WR); }
};

}; // namespace sockets

#endif