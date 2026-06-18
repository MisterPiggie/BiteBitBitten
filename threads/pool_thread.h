#ifndef THREAD_POOL_H
#define THREAD_POOL_H
#include <pthread.h>
#include <stdbool.h>
#include "../types/types.h"




void *worker(void *arg);

void threadpool_init(CL_threadpool *pool);
void threadpool_push(CL_threadpool *pool, task_fn fn, void *arg);
void threadpool_destroy(CL_threadpool *pool);
void notify_pipe_readable(EV_loop *loop);

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

typedef struct {
    TR_torrent *torrent;
    TR_peer    *peer;
    uint32_t   piece_idx;
    uint32_t   data_len;
    int        pipefd;
    uint8_t    *data;   
} piece_task;


typedef struct 
{
    CL_session  *session;
    TR_torrent  *torrent;
    NET_tracker *tracker;
    int         pipefd;
} HTTP_announce_args;

typedef struct 
{
    NET_tracker *tracker;
    TR_torrent  *torrent;

    bool        success;
    int         interval;

    uint8_t     *peers;
    int         peers_len;
} HTTP_announce_result;


typedef enum 
{
    NOTIFY_DNS_RESOLVE,
    NOTIFY_SHA1_VERIFY,
    NOTIFY_DISK_WRITE,
    NOTIFY_HTTP_RESPONSE,
} notify_type;

typedef struct {
    TR_torrent *torrent;
    TR_peer    *peer;
    uint32_t   piece_idx;
    bool       success;
} SHA1_verify_result;

typedef struct {
    TR_torrent *torrent;
    TR_peer    *peer;
    uint32_t   piece_idx;
    bool       success;
} disk_write_result;

typedef struct
{
    uint8_t *buf;
    size_t  len;
} CURL_buf;

typedef struct 
{
    notify_type type;
    union 
    {
        DNS_resolve_result dns;
        HTTP_announce_result http;
        SHA1_verify_result sha1;
        disk_write_result disk;
    };
} notify_result;


void task_resolve_DNS(void *arg);
void task_HTTP_announce(void *arg);
void task_sha1_verify(void *arg);
void task_disk_write(void *arg);

size_t curl_write_cb(void *data, size_t size, size_t nmemb, void *userp);
    
void handle_dns_result(EV_loop *loop, DNS_resolve_result *result);
void handle_disk_result(EV_loop *loop, disk_write_result *result);
void handle_HTTP_response(EV_loop *loop, HTTP_announce_result *result);
void handle_sha1_result(EV_loop *loop, SHA1_verify_result *result);
bool write_piece_to_disk(TR_torrent *tr, uint32_t piece_idx, uint8_t *data, uint32_t data_len);


#endif
