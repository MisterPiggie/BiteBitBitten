#include <netinet/in.h>
#include <stdint.h>
#include <event2/event.h>
#include "../types/types.h"

#define  DEFAULT_PORT 6881
#define UDP_MAGIC_NUMBER 0x41727101980


static const char charset[] = "abcdefghijklmnopqrstuvwxyz"
                              "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                              "0123456789";
static const int charset_len = sizeof(charset) - 1; 

static const char peer_id_version[] = "-BB1313-";
static const int peer_id_version_len = sizeof(peer_id_version) - 1; 

typedef enum {
    UDP_ACTION_CONNECT  = 0,
    UDP_ACTION_ANNOUNCE = 1,
    UDP_ACTION_SCRAPE   = 2,
    UDP_ACTION_ERROR    = 3,
} UDP_action;

void generate_peer_id(unsigned char peer_id[20]);
void HTTP_url_encode_hash(const uint8_t *hash_info, char *out);



uint32_t get_random_u32(void);

int tracker_connect(int sock, struct sockaddr_in *trackerr, uint32_t *tid_out);


int tracker_string_to_NET_tracker(char *str, NET_tracker *track);

int UDP_send_connect_req(CL_announcer *ann, NET_tracker *track);
int UDP_sendto(int sock, NET_tracker *track, const uint8_t *buf, size_t len);
void UDP_construct_connect_body(uint8_t buf[16], uint32_t id);
int UDP_recv(int sock, uint8_t *buf, size_t len);
int UDP_recv_connect_req(CL_announcer *ann, NET_tracker *track);
