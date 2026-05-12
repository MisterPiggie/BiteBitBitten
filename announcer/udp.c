#include <endian.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include "announcer.h"


int UDP_send_connect_req(CL_announcer *ann, NET_tracker *track)
{
    track->connection_id = get_random_u32();
    uint8_t buf[16];

    UDP_construct_connect_body(buf, track->connection_id);
    
    if(UDP_sendto(ann->udp_socket, track, buf, sizeof(buf)) != 0)
        return -1;
    return 0;
}


int UDP_recv_connect_req(CL_announcer *ann, NET_tracker *track)
{
    int n;
    uint8_t buf[16];
    uint32_t action, trans_id;
    uint64_t conn_id;

    n = UDP_recv(ann->udp_socket, buf, sizeof(buf));

    if (n < 16)
        return -1;

    action = ntohl(*(uint32_t *)(buf));
    trans_id = ntohl(*(uint32_t *)(buf + 4));
    conn_id = ntohl(*(uint64_t *)(buf + 8));

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
