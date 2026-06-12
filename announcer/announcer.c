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

    printf("CONNECTING TO PEER\n");

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

    peer->context.peer = peer;
    peer->context.tr = tr;
    peer->peer_state = CONNECTING;

    struct epoll_event ev = {
        .events   = EPOLLOUT,
        .data.ptr = &peer->context,  
    };

    epoll_ctl(loop->epollfd, EPOLL_CTL_ADD, peer->sock, &ev);
}

void on_peer_connected(EV_loop *loop, TR_peer_ctx *context)
{
    TR_peer *peer  = context->peer;
    TR_torrent *torrent  = context->tr;
    int idx;
    int err;
    socklen_t len = sizeof(err);

    getsockopt(peer->sock, SOL_SOCKET, SO_ERROR, &err, &len);

    if (err != 0)
    {
        peer->peer_state = NOT_CONNECTED;
        peer->failed_tries++;
        epoll_ctl(loop->epollfd, EPOLL_CTL_DEL, peer->sock, NULL);
        close(peer->sock);
        peer->sock = -1;
        return;
    }

    idx = torrent->swarm->peers_count;
    peer->bitfield = (torrent->swarm->bitfield_slab + torrent->swarm->bytes_count * idx);
    peer->piece_buf = (torrent->swarm->piece_buf_slab + torrent->info->piece_length * idx);

    torrent->swarm->peers[idx] = peer;
    torrent->swarm->peers_count++;

    peer->peer_state = HANDSHAKING;
    send_handshake(peer, torrent, loop->session->peer_id);

    struct epoll_event ev =
    {
        .events = EPOLLIN,
        .data.ptr = &peer->context,
    };

    printf("PEER CONNECTED\n");
    epoll_ctl(loop->epollfd, EPOLL_CTL_MOD, peer->sock, &ev);
}

void on_peer_readable(EV_loop *loop, TR_peer_ctx *context)
{
    TR_peer *peer  = context->peer;
    TR_torrent *torrent  = context->tr;

    printf("PEER READABLE\n");
    switch (peer->peer_state)
    {
        case HANDSHAKING:
            handle_handshake(loop, peer, torrent);
            break;
        case CONNECTED:
            handle_message(loop, peer, torrent);
            break;
        case BANNED: case NOT_CONNECTED: case CONNECTING:
            break;
    }
}

void handle_handshake(EV_loop *loop, TR_peer *peer, TR_torrent *tr)
{
    uint8_t handshake[68];
    ssize_t n = recv(peer->sock, handshake, 68, 0);
    printf("PEER HANDSHAKE\n");

    if (n != 68)
    {
        disconnect_peer(loop, peer);
        return;
    }

    if (handshake[0] != 19 || memcmp(handshake + 1, "BitTorrent protocol", 19) != 0)
    {
        disconnect_peer(loop, peer);
        return;
    }

    if (memcmp(handshake + 28, tr->info->info_hash, 20) != 0)
    {
        disconnect_peer(loop, peer);
        return;
    }

    memcpy(loop->session->peer_id, handshake + 48, 20);

    peer->peer_state = CONNECTED;

}

void handle_message(EV_loop *loop, TR_peer *peer, TR_torrent *tr)
{
    uint32_t msg_len;
    recv(peer->sock, &msg_len, 4, 0);
    msg_len = ntohl(msg_len);

    if (msg_len == 0)
        return;

    uint8_t msg_id;
    recv(peer->sock, &msg_id, 1, 0);

    switch (msg_id)
    {
        case 0:  
            peer->bitmask_state |= PEER_CHOKING_US;
            printf("PEER CHOKING US\n");
            break;

        case 1:  
            peer->bitmask_state &= ~PEER_CHOKING_US;
            printf("PEER UNCHOKING US\n");
            assign_piece_to_peer(peer, tr);
            break;

        case 2:  
            peer->bitmask_state |= PEER_INTERESTED_IN_US;
            printf("PEER INTERESTED US\n");
            break;

        case 3:  
            peer->bitmask_state &= ~PEER_INTERESTED_IN_US;
            printf("PEER UNINTERESTED US\n");
            break;

        case 4:  
        {
            uint32_t piece_index;
            printf("PEER  HAVE\n");
            recv(peer->sock, &piece_index, 4, 0);
            piece_index = ntohl(piece_index);
            peer_set_piece(peer, piece_index);
            break;
        }

        case 5:  
        {
            int bitfield_len = msg_len - 1;
            recv(peer->sock, peer->bitfield, bitfield_len, 0);
            send_interested(peer);
            printf("PEER BITFIELD\n");
            break;
        }

        case 7:  
            printf("PEER PIECE\n");
            handle_piece(loop, peer, tr, msg_len);
            break;

        default:
        {
            uint8_t tmp[512];
            int remaining = msg_len - 1;
            while (remaining > 0)
            {
                int n = recv(peer->sock, tmp,
                             remaining < 512 ? remaining : 512, 0);
                remaining -= n;
            }
            break;
        }
    }
}

void send_handshake(TR_peer *peer, TR_torrent *tr, uint8_t *peer_id)
{
    uint8_t handshake[68];

    handshake[0] = 19;
    memcpy(handshake + 1,  "BitTorrent protocol", 19);
    memset(handshake + 20, 0, 8);                        
    memcpy(handshake + 28, tr->info->info_hash, 20);  
    memcpy(handshake + 48, peer_id, 20);

    send(peer->sock, handshake, 68, 0);
}

void disconnect_peer(EV_loop *loop, TR_peer *peer)
{
    TR_swarm *swarm = peer->context.tr->swarm;
    int i;

    epoll_ctl(loop->epollfd, EPOLL_CTL_DEL, peer->sock, NULL);
    close(peer->sock);

    peer->sock = -1;
    peer->current_piece = -1;
    peer->blocks_received = 0;
    peer->peer_state = NOT_CONNECTED;
    peer->failed_tries++;

    memset(peer->bitfield,  0, swarm->bytes_count);
    memset(peer->piece_buf, 0, swarm->piece_length);


    for (i = 0; i < swarm->peers_count; i++)
    {
        if (swarm->peers[i] == peer)
        {
            swarm->peers[i] = swarm->peers[--swarm->peers_count];
            break;
        }
    }
    
    return;
}

void peer_set_piece(TR_peer *peer, uint64_t piece_idx)
{
    peer->bitfield[piece_idx / 8] |= (1 << (7 - piece_idx % 8));
}


void assign_piece_to_peer(TR_peer *peer, TR_torrent *torrent)
{
    for (int i = 0; (uint64_t) i < torrent->info->pieces_count; i++)
    {
        if (torrent->bitfield[i / 8] & (1 << (7 - i % 8)))
            continue;
        if (torrent->requested_pieces[i / 8] & (1 << (7 - i % 8)))
            continue;
        if (!(peer->bitfield[i / 8] & (1 << (7 - i % 8))))
            continue;

        torrent->requested_pieces[i / 8] |= (1 << (7 - i % 8));
        peer->current_piece          = i;
        peer->blocks_received        = 0;

        request_all_blocks(peer, torrent, i);
        return;
    }
}

void request_all_blocks(TR_peer *peer, TR_torrent *torrent, int piece_idx)
{
    uint32_t piece_size;
    uint32_t offset;
    uint32_t block_len;

    if ((uint32_t) piece_idx == torrent->info->pieces_count - 1)
        piece_size = torrent->info->total_size % torrent->info->piece_length;
    else
        piece_size = torrent->info->piece_length;

    for (offset = 0; offset < piece_size; offset += BLOCK_SIZE)
    {
        block_len = piece_size - offset;
        if (block_len > BLOCK_SIZE)
            block_len = BLOCK_SIZE;

        send_request(peer, piece_idx, offset, block_len);
    }
}

void send_request(TR_peer *peer, uint32_t piece_idx, uint32_t offset, uint32_t length)
{
    uint8_t buf[17];

    *(uint32_t *)(buf)      = htonl(13);         
    *(uint8_t  *)(buf + 4)  = 6;                 
    *(uint32_t *)(buf + 5)  = htonl(piece_idx);
    *(uint32_t *)(buf + 9)  = htonl(offset);
    *(uint32_t *)(buf + 13) = htonl(length);

    send(peer->sock, buf, 17, 0);
}

void send_interested(TR_peer *peer)
{
    uint8_t buf[5];

    *(uint32_t *)(buf + 0) = htonl(1);  
    *(uint8_t  *)(buf + 4) = 2;         

    send(peer->sock, buf, 5, 0);
}

void handle_piece(EV_loop *loop, TR_peer *peer, TR_torrent *tr, uint32_t msg_len)
{
    uint32_t piece_idx;
    uint32_t offset;
    uint32_t block_len = msg_len - 9;  
    uint8_t tmp[BLOCK_SIZE];
    uint32_t block_idx;

    recv(peer->sock, &piece_idx, 4, MSG_WAITALL);
    recv(peer->sock, &offset,    4, MSG_WAITALL);

    piece_idx = ntohl(piece_idx);
    offset    = ntohl(offset);

    if (piece_idx != (uint32_t)peer->current_piece)
    {
        recv(peer->sock, tmp, block_len, MSG_WAITALL);
        return;
    }

    recv(peer->sock, peer->piece_buf + offset, block_len, MSG_WAITALL);

    block_idx = offset / BLOCK_SIZE;
    peer->blocks_received |= (1 << block_idx);

    uint32_t piece_size = (piece_idx == (uint32_t)tr->info->pieces_count - 1)
        ? tr->info->total_size % tr->info->piece_length
        : tr->info->piece_length;

    uint32_t total_blocks = (piece_size + BLOCK_SIZE - 1) / BLOCK_SIZE;
    uint32_t all_received = (1 << total_blocks) - 1;

    if ((uint32_t) peer->blocks_received == all_received)
    {
        piece_task *task = arena_push_struct(&tr->arena, piece_task);
        task->torrent   = tr;
        task->peer      = peer;
        task->piece_idx = piece_idx;
        task->data_len  = piece_size;
        task->pipefd    = loop->notify_pipe[1];
        memcpy(task->data, peer->piece_buf, piece_size);

        threadpool_push(loop->pool, task_sha1_verify, task);

        peer->current_piece   = -1;
        peer->blocks_received = 0;}
}

void send_have(TR_peer *peer, uint32_t piece_idx)
{
    uint8_t buf[9];

    *(uint32_t *)(buf + 0) = htonl(5);           
    *(uint8_t  *)(buf + 4) = 4;                 
    *(uint32_t *)(buf + 5) = htonl(piece_idx);

    send(peer->sock, buf, 9, 0);
}
