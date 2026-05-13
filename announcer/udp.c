#include <endian.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include "announcer.h"


int UDP_send_connect_req(int sock, NET_tracker *track)
{
    track->connection_id = get_random_u32();
    uint8_t buf[16];

    UDP_construct_connect_body(buf, track->connection_id);
    
    if(UDP_sendto(sock, track, buf, sizeof(buf)) != 0)
        return -1;
    return 0;
}

int UDP_send_announce_req(CL_session *session, TR_torrent *torrent, UDP_request *req)
{
    uint64_t connection_id, downloaded, left, uploaded;
    uint32_t action, txid, event, ip, key;
    int32_t  num_want;
    uint16_t port;

    uint8_t buf[98];
    memset(buf, 0, sizeof(buf));

    req->action = UDP_ACTION_ANNOUNCE;
    req->event  = 2;

    connection_id = htobe64(req->connection_id);
    action        = htonl(req->action);
    txid          = htonl(req->transction_id);
    downloaded    = htobe64(req->downloaded);
    uploaded      = htobe64(req->uploaded);
    left          = htobe64(req->left);
    event         = htonl(req->event);
    ip            = htonl(req->ip);
    key           = htonl(req->key);
    num_want      = htonl(req->num_want);
    port          = htons(req->port);

    memcpy(buf, &connection_id, 8);
    memcpy(buf + 8, &action, 4);
    memcpy(buf + 12, &txid, 4);
    memcpy(buf + 16, req->hash_info, 20);
    memcpy(buf + 36, session->peer_id, 20);
    memcpy(buf + 56, &downloaded, 8);
    memcpy(buf + 64, &left, 8);
    memcpy(buf + 72, &uploaded, 8);
    memcpy(buf + 80, &event, 4);
    memcpy(buf + 84, &ip, 4);
    memcpy(buf + 88, &key, 4);
    memcpy(buf + 92, &num_want, 4);
    memcpy(buf + 96, &port, 2);
    
    if (UDP_sendto(session->udp_socket, &torrent->tracks[torrent->active_tracker_idx], buf, sizeof(buf)) != 0)
        return -1;

    return  0;
}

int UDP_recv_announce_resp(int sock)
{
    int i, len, peer_count;
    uint8_t buf[2048];
    uint32_t action, txid, interval, leechers, seeders;

    len = UDP_recv(sock, buf, sizeof(buf));

    if (len < 0) {
        perror("recvfrom");  // prints the actual reason
        return -1;
    }

    if (len >= 8) {
        uint32_t action;
        memcpy(&action, buf, 4);
        if (ntohl(action) == UDP_ACTION_ERROR) {
            printf("tracker error: %.*s\n", len - 8, buf + 8);
            return -1;
        }
    }

    if (len < 16) {
        fprintf(stderr, "short response: %d bytes\n", len);
        return -1;
    }

    if (len < 20)
    {
        printf("Len < than 20\n");
        return -1;
    }

    memcpy(&action, buf, 4);
    action = ntohl(action);

    memcpy(&txid, buf + 4, 4);
    txid = ntohl(txid);

    memcpy(&interval, buf + 8, 4);
    interval = ntohl(interval);

    memcpy(&leechers, buf + 12, 4);
    leechers = ntohl(leechers);

    memcpy(&seeders, buf + 16, 4);
    seeders = ntohl(seeders);

    if (action != UDP_ACTION_ANNOUNCE)
    {
        printf("Action is not UDP_ACTION_ANNOUNCE\n");
        return -1;
    }

    peer_count = (len - 20) / 6;
    for (i = 0; i < peer_count; i++)
    {
        uint8_t *peer = buf + 20 + i * 6;

        uint32_t ip;
        uint16_t port;
        memcpy(&ip,   peer + 0, 4);
        memcpy(&port, peer + 4, 2);

        ip   = ntohl(ip);
        port = ntohs(port);

        printf("%d.%d.%d.%d:%d\n",
            (ip >> 24) & 0xff,
            (ip >> 16) & 0xff,
            (ip >>  8) & 0xff,
            (ip      ) & 0xff,
            port);
    }

    return 0;

}


int UDP_recv_connect_req(int sock, NET_tracker *track)
{
    int n;
    uint8_t buf[16];
    uint32_t action, trans_id;
    uint64_t conn_id;

    n = UDP_recv(sock, buf, sizeof(buf));

    if (n < 16)
        return -1;

    action = ntohl(*(uint32_t *)(buf));
    trans_id = ntohl(*(uint32_t *)(buf + 4));
    conn_id = be64toh(*(uint64_t *)(buf + 8));

    if (action != UDP_ACTION_CONNECT)
        return -1;

    if (trans_id != track->connection_id)
        return -1;

    track->connection_id = conn_id;

    return 0;
}

void UDP_construct_connect_body(uint8_t buf[16], uint32_t id)
{
    uint64_t conn_id = htobe64(UDP_MAGIC_NUMBER);
    uint32_t action = htonl(UDP_ACTION_CONNECT);
    uint32_t network_id = htonl(id);

    memcpy(buf, &conn_id, 8);
    memcpy(buf + 8, &action, 4);
    memcpy(buf + 12, &network_id, 4);
}

int UDP_sendto(int sock, NET_tracker *track, const uint8_t *buf, size_t len)
{
    struct addrinfo hints = {0};
    struct addrinfo *res;
    char port_str[6];
    ssize_t sent;

    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_DGRAM;

    snprintf(port_str, sizeof(port_str), "%u", track->port); 

    if (getaddrinfo(track->host, port_str, &hints, &res) != 0)
    {
        return -1;
    }

    sent = sendto(sock, buf, len, 0, res->ai_addr, res->ai_addrlen);
    freeaddrinfo(res);

    if (sent != (ssize_t)len)
    {
        return -1;
    }
    return 0;
}

int UDP_recv(int sock, uint8_t *buf, size_t len)
{
    ssize_t recieved = recvfrom(sock, buf, len, 0, NULL, NULL);
    if (recieved < 0)
    {
        return -1;
    }

    return (int)recieved;
}

void construct_UDP_request(UDP_request *req, TR_torrent *torrent)
{
    req->transction_id = get_random_u32();
    req->connection_id = torrent->tracks[torrent->active_tracker_idx].connection_id;
    req->downloaded = 0;
    req->uploaded = 0;
    req->left = torrent->info->total_size;
    req->ip = 0;
    req->port = DEFAULT_PORT;
    req->num_want = -1;
    req->key = torrent->tracks[torrent->active_tracker_idx].key;
    req->hash_info = torrent->info->info_hash;
}
