#include "pool_thread.h"

#include <netdb.h>
#include <unistd.h>

void task_resolve_DNS(void *arg)
{
    char port_str[6];
    DNS_resolve_args args = *(DNS_resolve_args *)arg;

    DNS_resolve_result result = 
    {
        .torrent = args.torrent,
        .tracker = args.tracker,
        .success = false,
        .ip      = 0,
    };

    struct addrinfo hints = {0};
    struct addrinfo *res;

    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_DGRAM;

    snprintf(port_str, sizeof(port_str), "%u", args.tracker->port);

    if (getaddrinfo(args.tracker->host, port_str, &hints, &res) == 0)
    {
        result.ip = ((struct sockaddr_in *) res->ai_addr)->sin_addr.s_addr;
        result.success = true;
        freeaddrinfo(res);
    }
    
    notify_result notify = {
        .type = NOTIFY_DNS_RESOLVE,
        .dns = result,
    };

    write(args.pipefd, &notify, sizeof(notify));
}

void notify_pipe_readable(EV_loop *loop)
{
    notify_result result;
    read(loop->notify_pipe[0], &result, sizeof(result));

    switch (result.type)
    {
        case NOTIFY_DNS_RESOLVE:
            handle_dns_result(loop, &result.dns);
            break;

        case NOTIFY_SHA1_VERIFY:
            // handle_sha1_result(loop, &result.sha1);
            break;

        case NOTIFY_DISK_WRITE:
            // handle_disk_result(loop, &result.disk);
            break;
    }
}

void handle_dns_result(EV_loop *loop, DNS_resolve_result *result)
{
    if (!result->success)
    {
        result->tracker->state = TRACKER_FAILED;
        return;
    }

    result->tracker->ip = result->ip;
    result->tracker->state = TRACKER_IDLE;
}
