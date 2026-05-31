#include "announcer.h"
#include <sys/socket.h>
#include <netinet/in.h>

void make_UDP_connect(TR_torrent *tr, EV_loop *loop)
{
    NET_tracker *tracker = &tr->tracks[tr->active_tracker_idx];
    UDP_request *req;

    uint8_t buf[16];
    uint64_t magic = htobe64(UDP_MAGIC_NUMBER);
    uint32_t action = htonl(0);
    uint32_t transaction_id = get_random_u32();
    
    memcpy(buf, &magic, 8);
    memcpy(buf + 8, &action, 4);
    memcpy(buf + 12, &transaction_id, 4);

    struct sockaddr_in addr = 
    {
        .sin_family      = AF_INET,
        .sin_addr.s_addr = tracker->ip,
        .sin_port        = htons(tracker->port),
    };

    sendto(loop->udp_socket, buf, 16, 0, (struct sockaddr *)&addr, sizeof(addr));
    req = &loop->udp_requests[loop->udp_requests_count++];

    req->transaction_id = transaction_id;
    req->sent_at        = time(NULL);
    req->torrent        = tr;
    req->tracker        = tracker;

    tracker->announce_sent_at   = time(NULL);
    

    return;
}

 void UDP_readable(EV_loop *loop)
{
    int i;
    int peers_length;
    ssize_t bytes;
    UDP_request *req = NULL;
    uint8_t buf[2048];
    bytes = recvfrom(loop->udp_socket, buf, sizeof(buf), 0, NULL, NULL);
    if (bytes < 8) 
        return;
    
    uint32_t action = ntohl(*(uint32_t *)buf);
    uint32_t transaction_id = ntohl(*(uint32_t *)(buf + 4));

    for( i = 0; i < loop->udp_requests_count; i++)
    {
        if (loop->udp_requests[i].transaction_id == transaction_id)
        {
            req = &loop->udp_requests[i];
            break;
        }
    }

    if (!req)
        return;

    switch (action)
    {
        case UDP_ACTION_CONNECT:
            req->tracker->connection_id = be64toh(*(uint64_t *)(buf + 8));
            req->tracker->state         = TRACKER_IDLE;
            make_UDP_announce(req->torrent, loop);
            req->tracker->state         = TRACKER_ANNOUNCING;
            req->tracker->announce_sent_at = time(NULL);
            break;

        case UDP_ACTION_ANNOUNCE:
            req->tracker->state = TRACKER_ALIVE;
            req->tracker->interval = ntohl(*(uint32_t *)(buf + 8));
            req->tracker->last_announce = time(NULL);

            peers_length = bytes - 20;
            parse_peers(buf + 20, peers_length, req->torrent);
            break;
        case UDP_ACTION_SCRAPE:
            break;
        case UDP_ACTION_ERROR:
            req->tracker->state = TRACKER_FAILED;
            break;
    }

    *req = loop->udp_requests[--loop->udp_requests_count];
}

void make_UDP_announce(TR_torrent *tr, EV_loop *loop)
{
    uint8_t buf[98] = {0};
    NET_tracker *tracker = &tr->tracks[tr->active_tracker_idx];
    uint32_t transaction_id = get_random_u32(); 
    uint32_t event;


    if (tr->state == TORRENT_DOWNLOADING) 
        event = 2;  
    else if (tr->state == TORRENT_SEEDING) 
        event = 1; 
    else
        event = 0;  

    *(uint64_t *)(buf + 0)  = htobe64(tracker->connection_id);
    *(uint32_t *)(buf + 8)  = htonl(1);  
    *(uint32_t *)(buf + 12) = htonl(transaction_id);
    memcpy(buf + 16, tr->info->info_hash, 20);
    memcpy(buf + 36, loop->session->peer_id, 20);
    *(uint64_t *)(buf + 56) = htobe64(tr->downloaded);
    *(uint64_t *)(buf + 64) = htobe64(tr->info->total_size - tr->downloaded);
    *(uint64_t *)(buf + 72) = htobe64(tr->uploaded);
    *(uint32_t *)(buf + 80) = htonl(event);  
    *(uint32_t *)(buf + 84) = htonl(0); 
    *(uint32_t *)(buf + 88) = htonl(get_random_u32());  
    *(int32_t  *)(buf + 92) = htonl(-1);  
    *(uint16_t *)(buf + 96) = htons(DEFAULT_PORT);

    UDP_sendto(loop->udp_socket, tracker, buf, sizeof(buf));

    UDP_request *req = &loop->udp_requests[loop->udp_requests_count++];
    req->transaction_id = transaction_id;
    req->tracker        = tracker;
    req->torrent        = tr;
    req->sent_at        = time(NULL);
}
