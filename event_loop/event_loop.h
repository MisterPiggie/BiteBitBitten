#ifndef EVENT_LOOP_H
#define EVENT_LOOP_H


#include "../types/types.h"


void init_EV_loop(EV_loop *loop, CL_session *session, CL_threadpool *pool);
void epoll_add(int epollfd, int fd, uint32_t events);
#endif
