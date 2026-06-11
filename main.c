#include <endian.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include "repl/repl.h"
#include "announcer/announcer.h"
#include "files/file_interactions.h"
#include "parser/parser.h"
#include "init/init_session.h"
#include "types/types.h"
#include "utils/str_utils.h"
#include "arena/arena.h"
#include "client/client.h"
#include "event_loop/event_loop.h"
#include "threads/pool_thread.h"

int main(void) {
    char  line[MAX_CHARS];
    char *argv[MAX_ARGS];
    Arena main_arena = arena_create(GB(1));
    CL_session session = {0};
    EV_loop loop;
    CL_threadpool pool;
    int n, i, running = 1;
    
    init_CL_session(&session, &main_arena);
    init_saved_torrents(&session);
    threadpool_init(&pool);
    init_EV_loop(&loop, &session, &pool);

    struct epoll_event ev =
    {
        .events = EPOLLIN,
        .data.fd = STDIN_FILENO,
    };

    epoll_ctl(loop.epollfd, EPOLL_CTL_ADD, STDIN_FILENO, &ev);
    printf("bbb=> ");
    fflush(stdout);

    struct epoll_event events[64];


    while (running) {
        n = epoll_wait(loop.epollfd, events, 64, -1);
        for (i = 0; i < n; i++)
        {
            int fd = events[i].data.fd;
            
            if (fd == STDIN_FILENO)
            {
                if (!fgets(line, sizeof(line), stdin))
                {
                    for (int i = 0; i < session.torrents_count; i++)
                    {
                        arena_destroy(&session.torrents[i]->arena);
                    }
                    arena_destroy(&main_arena);
                    break;
                }
                line[strcspn(line, "\n")] = '\0';

                if (line[0] == '\0') 
                    continue;                 

                if (strcmp(line, "exit") == 0) 
                {
                    running = 0;
                    break;
                }

                int argc = tokenize(line, argv, MAX_ARGS);
                dispatch(&session, argc, argv);

                printf("bbb=> ");
                fflush(stdout);
            } else if (fd == loop.client_timerfd) 
            {
                uint64_t exp;
                read(fd, &exp, 8);
                CL_client_tick(&loop);
            } else if (fd == loop.peer_timerfd) 
            {
                uint64_t exp;
                read(fd, &exp, 8);
                for (int j = 0; j < loop.session->torrents_count; j++)
                    ANN_refill_peers(&loop, loop.session->torrents[j]);
            } else if (fd == loop.udp_socket)
            {
                UDP_readable(&loop);
                continue;
            } else if (fd == loop.notify_pipe[0])
            {
                notify_pipe_readable(&loop);
                continue;
            } else
            {
                TR_peer_ctx *ctx = events[i].data.ptr;
                if (events[i].events & EPOLLOUT)
                    on_peer_connected(&loop, ctx);
                else if (events[i].events & EPOLLIN)
                    on_peer_readable(&loop, ctx);
            }
        }

    }
    for (int i = 0; i < session.torrents_count; i++)
    {
        arena_destroy(&session.torrents[i]->arena);
    }
    arena_destroy(&main_arena);
    threadpool_destroy(&pool);
    puts("Bye!");
    return 0;
}
    
    
