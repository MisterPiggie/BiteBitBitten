#include "announcer.h"
#include <sys/socket.h>
#include <netinet/in.h>

void make_UDP_connect(TR_torrent *tr, uint8_t *peer_id)
{
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
    };

    return;
}
