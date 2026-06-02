#include "announcer.h"
#include <stdlib.h>
#include <curl/curl.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h>
#include <stdio.h>
#include <errno.h>
#include <time.h>
#include <sys/random.h>
#include <sys/epoll.h>
#include "../threads/pool_thread.h"
#include "../event_loop/event_loop.h"


void ANN_announcer_tick(EV_loop *loop, TR_torrent *tr)
{
    int i;

    NET_tracker *tracker = &tr->tracks[tr->active_tracker_idx];
    switch (tracker->state)
    {
        case TRACKER_NOT_RESOLVED:
            ANN_resolve_tracker(tr, loop);
            tracker->state = TRACKER_RESOLVING;
            break;
        case TRACKER_RESOLVING:
            break;
        case TRACKER_IDLE:
            ANN_make_announce_req(tr, loop);
            tracker->state = TRACKER_ANNOUNCING;
            tracker->announce_sent_at = time(NULL);
            break;
        case TRACKER_ANNOUNCING:
            if (time(NULL) - tracker->announce_sent_at > 5)
            {
                tracker->state = TRACKER_FAILED;
                for (i = 0; i< loop->udp_requests_count; i++)
                    if (loop->udp_requests[i].tracker == tracker)
                    {
                        loop->udp_requests[i] = loop->udp_requests[--loop->udp_requests_count];
                        i--;
                    }

                if (++tr->active_tracker_idx >= tr->tracker_count)
                    tr->state = TORRENT_FAILED;
            }
            break;
        case TRACKER_ALIVE:
            if (time(NULL) - tracker->last_announce > tracker->interval)
            {
                ANN_make_announce_req(tr, loop);
                tracker->state = TRACKER_ANNOUNCING;
                tracker->announce_sent_at = time(NULL);
            }
            break;
        case TRACKER_FAILED:
            if (++tr->active_tracker_idx >= tr->tracker_count)
                tr->state = TORRENT_FAILED;
            break;
    }
}


void ANN_resolve_tracker(TR_torrent *tr, EV_loop *loop)
{
    DNS_resolve_args args = { 
        .torrent = tr,
        .tracker = &tr->tracks[tr->active_tracker_idx],
        .pipefd = loop->notify_pipe[1],
    };

    threadpool_push(loop->pool, task_resolve_DNS, &args);
}

void ANN_make_announce_req(TR_torrent *tr, EV_loop *loop)
{
    NET_tracker *tracker = &tr->tracks[tr->active_tracker_idx];
    if (strcmp(tracker->schema, "https") == 0 ||
        strcmp(tracker->schema, "http") == 0)
    {
        make_HTTP_announce(tr, loop);
        return;
    }

    if (strcmp(tracker->schema, "udp") == 0)
    {
        make_UDP_connect(tr, loop);
        return;
    }

    return;
}

void generate_peer_id(unsigned char peer_id[20])
{
   unsigned char buf[12];
   int i;
   for (i = 0; i < peer_id_version_len; i++)
   {
       peer_id[i] = peer_id_version[i];
   }

   if (getrandom(buf, sizeof(buf), 0) == sizeof(buf))
   {
       for (i = 0; i < (int) sizeof(buf); i++)
       {
           peer_id[i + peer_id_version_len] = charset[buf[i] % charset_len];
       }

       return;
   }

   FILE *f = fopen("/dev/urandom", "rb");
   if (f) 
   {
       if (fread(buf, sizeof(unsigned char), sizeof(buf), f) == 12)
       {
           for (i = 0; i < (int) sizeof(buf); i++)
           {
               peer_id[i + peer_id_version_len] = charset[buf[i] % charset_len];
           }
           fclose(f);
       }

       fclose(f);
   }
}

void HTTP_url_encode_hash(const uint8_t *hash_info, char *out)
{
    //OUT MUST BE 61 CHARS LONG
    int i;
    for (i = 0; i < 20; i++)
    {
        sprintf(out + i*3, "%%%02X", hash_info[i]);
    }
    out[60] = '\0';
}

uint32_t get_random_u32(void)
{
    uint32_t x;

    if (getrandom(&x, sizeof(x), 0) == sizeof(x))
        return x;

    FILE *f = fopen("/dev/urandom", "rb");
    if (f) 
    {
        if (fread(&x, sizeof(x), 1, f) == 1)
        {
            fclose(f);
            return x;
        }

        fclose(f);
    }
    return (uint32_t)(time(NULL) ^ getpid() ^ (uintptr_t)&x);
}

 

int tracker_string_to_NET_tracker(Arena *arena, char *url, NET_tracker *track)
{
    char *schema_end;
    char *host_start;
    char *path_start;
    char *colon;

    schema_end = strstr(url, "://");
    if (!schema_end)
        return -1;

    track->schema = arena_push_strn(arena, url, schema_end - url);
    host_start = schema_end + 3;

    colon = strchr(host_start, ':');
    if (!colon)
        return -1;

    track->host = arena_push_strn(arena, host_start, colon - host_start);

    track->port = atoi(colon + 1);

    path_start = strchr(colon + 1, '/');
    track->path = path_start ? arena_push_str(arena, path_start) : arena_push_str(arena, "/");

    if (strcmp(track->schema, "udp") == 0)
        track->state = TRACKER_NOT_RESOLVED;  

    else if (strcmp(track->schema, "http")  == 0 ||
            strcmp(track->schema, "https") == 0)
        track->state = TRACKER_IDLE;        

    track->ip = 0;




    return 0;
}

void parse_peers(uint8_t *peers, int peers_length, TR_torrent *tr)
{
    int i;
    uint32_t ip;
    uint16_t port;

    if (peers_length < 6) 
        return;

    for (i = 0 ; i + 6 <= peers_length; i += 6)
    {
        if (tr->swarm->pool_count >= 200)
            continue;
        ip = *(uint32_t *)(peers + i);
        port = ntohs(*(uint16_t *)(peers + i + 4));

        if (port == 0)
            continue;

        peer_pool_add(tr, ip, port);
    }
}

void peer_pool_add(TR_torrent *tr, uint32_t ip, uint16_t port)
{
    for (int i = 0; i < tr->swarm->pool_count; i++)
        if (tr->swarm->peer_pool[i]->ip == ip &&
            tr->swarm->peer_pool[i]->port == port)
            return;

    TR_peer *peer = tr->swarm->peer_pool[tr->swarm->pool_count++]; 
    peer->ip = ip;
    peer->port = port;
    peer->peer_state = NOT_CONNECTED;
    printf("Peer: %lu:%u\n", (unsigned long) ip, port);
}

void ANN_refill_peers(EV_loop *loop, TR_torrent *tr)
{
    TR_peer *peer;
    while (tr->swarm->peers_count < 50 && tr->swarm->pool_count > 0)
    {
        peer = pick_peer(tr->swarm);
        connect_to_peer(peer, loop, tr);
    }
}

TR_peer *pick_peer(TR_swarm *swarm)
{
    int i;
    TR_peer *peer;

    for (i = 0; i < swarm->pool_count; i++)
    {
        peer = swarm->peer_pool[i];
        if (peer->peer_state == CONNECTED)
            continue;

        if (time(NULL) - peer->last_tried < 300)
            continue;

        if (peer->failed_tries >= 3)
        {
            swarm->peer_pool[i] = swarm->peer_pool[--swarm->pool_count];
            i--;
            continue;
        }

        return peer;
    }
    
    return peer;
}

void connect_to_peer(TR_peer *peer, EV_loop *loop, TR_torrent *tr)
{
    int ret;
    int fd = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0);
    peer->sock = fd;


    struct sockaddr_in peer_addr = 
    {
        .sin_family = AF_INET,
        .sin_addr.s_addr = peer->ip,
        .sin_port = htons(peer->port),
    };

    ret = connect(fd, (struct sockaddr *)&peer_addr, sizeof(peer_addr));
    if (ret < 0 && errno != EINPROGRESS)
    {
        close(fd);
        return;
    }

    epoll_add(loop->epollfd, fd, EPOLLOUT);
}
