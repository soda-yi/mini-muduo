#ifndef _NETWORK_HH_
#define _NETWORK_HH_

#include <arpa/inet.h>
#include <netinet/in.h>
#include <string>
#include <string_view>

class EndPoint
{
public:
    EndPoint() noexcept {}
    /**
     * @brief 构造EndPoint
     * 
     * @param ipAddr 字符串类型的ip地址
     * @param port host端的port
     */
    EndPoint(std::string_view ipAddr, in_port_t port) noexcept
        : ipAddr_{inet_addr(ipAddr.data())}, port_{htons(port)}
    {
    }
    /**
     * @brief 构造EndPoint
     * 
     * @param ipAddr net端的ip地址
     * @param port net端的port
     */
    EndPoint(in_addr_t ipAddr, in_port_t port) noexcept
        : ipAddr_{ipAddr}, port_{port}
    {
    }
    /**
     * @brief 获取字符串类型的ip地址
     * 
     * @return std::string_view ip地址
     * 
     * @warning 调用后虚尽快复制副本，下次调用时会覆盖
     */
    std::string_view GetIpAddrString() const noexcept
    {
        return inet_ntoa({ipAddr_});
    }
    /**
     * @brief 获取net端的ip地址
     * 
     * @return in_addr_t net端的ip地址
     */
    in_addr_t GetIpAddr() const noexcept
    {
        return ipAddr_;
    }
    /**
     * @brief 获取host端的port
     * 
     * @return in_addr_t host端的port
     */
    in_addr_t GetPortH() const noexcept
    {
        return ntohs(port_);
    }
    /**
     * @brief 获取net端的port
     * 
     * @return in_addr_t net端的port
     */
    in_addr_t GetPort() const noexcept
    {
        return port_;
    }

private:
    in_addr_t ipAddr_;
    in_port_t port_;
};

#endif