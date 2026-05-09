#include <endian.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include "announcer.h"

int UDP_send_connect_req(int sock, struct sockaddr *addr)
{
    UDP_req_connect_packed req;
    req.connection_id = htobe64(UDP_MAGIC_NUMBER);
    req.action = htobe64(0);
    req.transction_id = htobe64(get_random_u32());

    return sendto(sock, &req, sizeof(req), 0, addr, sizeof(*addr));
}

int UDP_resolve_tracker(NET_tracker *track, struct sockaddr_in *addr)
{
    struct addrinfo hints = {0};
    struct addrinfo *res;
    char port_str[6];
    int err;

    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_DGRAM;

    snprintf(port_str, sizeof(port_str), "%u", track->port); 

    err = getaddrinfo(track->host, port_str, &hints, &res);
    if (err != 0)
    {
        return -1;
    }

    memcpy(addr, res->ai_addr, sizeof(struct sockaddr_in));
    freeaddrinfo(res);
    return 0;
}
