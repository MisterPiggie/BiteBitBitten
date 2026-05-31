#ifndef THREAD_POOL_H
#define THREAD_POOL_H
#include <pthread.h>
#include <stdbool.h>
#include "../types/types.h"




void *worker(void *arg);

void threadpool_init(CL_threadpool *pool);
void threadpool_push(CL_threadpool *pool, task_fn fn, void *arg);
void threadpool_destroy(CL_threadpool *pool);

//Tasks


typedef struct 
{
    TR_torrent  *torrent;
    NET_tracker *tracker;
    int         pipefd;
} DNS_resolve_args;

typedef struct 
{
    NET_tracker *tracker;
    TR_torrent  *torrent;
    bool        success;
    uint32_t    ip;
} DNS_resolve_result;

typedef enum 
{
    NOTIFY_DNS_RESOLVE,
    NOTIFY_SHA1_VERIFY,
    NOTIFY_DISK_WRITE,
} notify_type;

typedef struct 
{
    notify_type type;
    union 
    {
        DNS_resolve_result dns;
        // SHA1_verify_result sha1;
        // disk_write_result disk;
    };
} notify_result;

void task_resolve_DNS(void *arg);

void handle_dns_result(EV_loop *loop, DNS_resolve_result *result);

#endif
