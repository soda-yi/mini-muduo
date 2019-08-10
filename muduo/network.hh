#ifndef _NETWORK_HH_
#define _NETWORK_HH_

#include <cerrno>
#include <cstring>
#include <stdexcept>
#include <string>

struct EndPoint {
    std::string ip_addr;
    unsigned short port;
};

#endif