#include <sys/epoll.h>
#include <sys/timerfd.h>
#include <sys/socket.h>
#include "event_loop.h"

void init_EV_loop(EV_loop *loop, CL_session *session)
{
    loop->session = session;
    loop->epollfd = epoll_create1(0);
     
    loop->client_timerfd = timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK);

    struct itimerspec ts =
    {
        .it_interval = {1, 0},
        .it_value = {1, 0},
    };
    timerfd_settime(loop->client_timerfd, 0, &ts, NULL);
    epoll_add(loop->epollfd, loop->client_timerfd, EPOLLIN);


    loop->peer_timerfd = timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK);
    timerfd_settime(loop->peer_timerfd, 0, &ts, NULL);
    epoll_add(loop->epollfd, loop->peer_timerfd, EPOLLIN);

    loop->udp_socket = socket(AF_INET, SOCK_DGRAM | SOCK_NONBLOCK, 0);
    epoll_add(loop->epollfd, loop->udp_socket, EPOLLIN);

    return;
}

void epoll_add(int epollfd, int fd, uint32_t events)
{
    struct epoll_event ev =
    {
        .events = events,
        .data.fd = epollfd,
    };
    epoll_ctl(epollfd, EPOLL_CTL_ADD, fd, &ev);
}
