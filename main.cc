#include <arpa/inet.h>
#include <fcntl.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <unistd.h>

#include <cerrno>
#include <cstring>
#include <iostream>

using std::cout;
using std::endl;

int createAndListen()
{
    int on = 1;
    int listenfd = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in servaddr;
    fcntl(listenfd, F_SETFL, O_NONBLOCK); //no-block io
    setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(11111);

    if (-1 == bind(listenfd, (struct sockaddr *)&servaddr, sizeof(servaddr))) {
        cout << "bind error, errno:" << errno << endl;
    }

    constexpr int backlog = 10;
    if (-1 == listen(listenfd, backlog)) {
        cout << "listen error, errno:" << errno << endl;
    }

    return listenfd;
}

int main()
{
    constexpr int kMaxEvents = 500, kMaxLength = 100;
    int connfd, sockfd;
    int readlength;
    char line[kMaxLength];
    struct sockaddr_in cliaddr;
    socklen_t clilen = sizeof(cliaddr);

    int epollfd = epoll_create1(1);
    if (epollfd < 0) {
        cout << "epoll_create error, error:" << epollfd << endl;
    }

    int listenfd = createAndListen();
    struct epoll_event ev, events[kMaxEvents];
    ev.data.fd = listenfd;
    ev.events = EPOLLIN;
    epoll_ctl(epollfd, EPOLL_CTL_ADD, listenfd, &ev);

    for (;;) {
        int fds = epoll_wait(epollfd, events, kMaxEvents, -1);
        if (fds == -1) {
            cout << "epoll_wait error, errno:" << errno << endl;
            break;
        }

        for (int i = 0; i < fds; ++i) {
            if (events[i].data.fd == listenfd) {
                connfd = accept(listenfd, (sockaddr *)&cliaddr, (socklen_t *)&clilen);
                if (connfd > 0) {
                    cout << "new connection from "
                         << "[" << inet_ntoa(cliaddr.sin_addr)
                         << ":" << ntohs(cliaddr.sin_port) << "]"
                         << " accept socket fd:" << connfd
                         << endl;
                } else {
                    cout << "accept error, connfd:" << connfd
                         << " errno:" << errno << endl;
                }

                fcntl(connfd, F_SETFL, O_NONBLOCK);
                ev.data.fd = connfd;
                ev.events = EPOLLIN;
                if (epoll_ctl(epollfd, EPOLL_CTL_ADD, connfd, &ev) == -1) {
                    cout << "epoll_ctrl error, errno:" << errno << endl;
                }
            } else if (events[i].events & EPOLLIN) {
                if ((sockfd = events[i].data.fd) < 0) {
                    cout << "EPOLLIN sockfd < 0 error " << endl;
                    continue;
                }
                bzero(line, kMaxLength);
                if ((readlength = read(sockfd, line, kMaxLength)) < 0) {
                    if (errno == ECONNRESET) {
                        cout << "ECONNREST closed socket fd:" << events[i].data.fd << endl;
                        close(sockfd);
                    }
                } else if (readlength == 0) {
                    cout << "read 0 closed socket fd:" << events[i].data.fd << endl;
                    close(sockfd);
                } else {
                    if (write(sockfd, line, readlength) != readlength) {
                        cout << "error: not finished one time" << endl;
                    }
                }
            }
        }
    }
}