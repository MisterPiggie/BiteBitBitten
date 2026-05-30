#ifndef EVENT_LOOP_H
#define EVENT_LOOP_H


#include "../types/types.h"

typedef struct
{
    CL_session  *session;

    int         epollfd;
    int         client_timerfd;
    int         peer_timerfd;

    int         udp_socket;
} EV_loop;

void init_EV_loop(EV_loop *loop, CL_session *session);
void epoll_add(int epollfd, int fd, uint32_t events);
#endif
