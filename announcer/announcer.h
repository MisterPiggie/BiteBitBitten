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



typedef struct __attribute__((packed)) {
    uint64_t connection_id;
    uint32_t action;
    uint32_t transction_id;
} UDP_req_connect_packed;

typedef struct {
    uint64_t connection_id;
    uint32_t action;
    uint32_t transction_id;
} UDP_req_connect;

typedef struct __attribute__((packed)) {
    uint64_t connection_id;
    uint32_t action;
    uint32_t transction_id;
    uint8_t  hash_info[20];
    uint8_t  peer_id[20];
    uint64_t downloaded;
    uint64_t left;
    uint64_t uploaded;
    uint32_t event;
    uint32_t ip;
    uint32_t key;
    int32_t  num_want;
    uint16_t port;
} UDP_req_announce_packed;

typedef struct {
    uint64_t connection_id;
    uint32_t action;
    uint32_t transction_id;
    uint8_t  hash_info[20];
    uint8_t  peer_id[20];
    uint64_t downloaded;
    uint64_t left;
    uint64_t uploaded;
    uint32_t event;
    uint32_t ip;
    uint32_t key;
    int32_t  num_want;
    uint16_t port;
} UDP_req_announce;

typedef struct {
    int                 UDP_sock;
    struct event_base   *ev_base;
} UDP_server;


uint32_t get_random_u32(void);

int tracker_connect(int sock, struct sockaddr_in *trackerr, uint32_t *tid_out);


int tracker_string_to_NET_tracker(char *str, NET_tracker *track);
