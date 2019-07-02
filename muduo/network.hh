#ifndef _NETWORK_HH_
#define _NETWORK_HH_

#include <string>

struct EndPoint {
    std::string ip_addr;
    unsigned short port;
};

#endif