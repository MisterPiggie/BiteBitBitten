#include <stdint.h>
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
} udp_req_connect;

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
} udp_req_announce;

uint32_t get_random_u32(void);
