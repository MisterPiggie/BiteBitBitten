#define  DEFAULT_PORT 6881

static const char charset[] = "abcdefghijklmnopqrstuvwxyz"
                              "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                              "0123456789";
static const int charset_len = sizeof(charset) - 1; 

static const char peer_id_version[] = "-BB1313-";
static const int peer_id_version_len = sizeof(peer_id_version) - 1; 

void generate_peer_id(unsigned char peer_id[20]);


