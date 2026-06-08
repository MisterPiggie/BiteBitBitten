#include <netinet/in.h>
#include <stdint.h>
#include <stdbool.h>
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


void ANN_announcer_tick(EV_loop *loop, TR_torrent *tr);
void ANN_refill_peers(EV_loop *loop, TR_torrent *tr);

TR_peer *pick_peer(TR_swarm *swarm);
void ANN_resolve_tracker(TR_torrent *tr, EV_loop *loop);
void ANN_make_announce_req(TR_torrent *tr, EV_loop *loop);
void make_HTTP_announce(TR_torrent *tr, EV_loop *loop);
void make_UDP_connect(TR_torrent *tr, EV_loop *loop);
void make_UDP_announce(TR_torrent *tr, EV_loop *loop);
void UDP_readable(EV_loop *loop);
void parse_peers(uint8_t *peers, int peers_length, TR_torrent *tr);
void peer_pool_add(TR_torrent *tr, uint32_t ip, uint16_t port);
void connect_to_peer(TR_peer *peer, EV_loop *loop, TR_torrent *tr);

uint32_t get_random_u32(void);

int tracker_connect(int sock, struct sockaddr_in *trackerr, uint32_t *tid_out);


int tracker_string_to_NET_tracker(Arena *arena, char *url, NET_tracker *track);

int UDP_send_connect_req(int sock, NET_tracker *track);
int UDP_sendto(int sock, NET_tracker *track, const uint8_t *buf, size_t len);
void UDP_construct_connect_body(uint8_t buf[16], uint32_t id);
int UDP_recv(int sock, uint8_t *buf, size_t len);
int UDP_recv_connect_req(int sock, NET_tracker *track);
int UDP_send_announce_req(CL_session *session, TR_torrent *torrent, UDP_request *req);
UDP_request *construct_UDP_request(TR_torrent *torrent);
int UDP_recv_announce_resp(int sock);
void on_peer_connected(EV_loop *loop, TR_peer_ctx *context);
void handle_handshake(EV_loop *loop, TR_peer *peer, TR_torrent *tr);
void handle_message(EV_loop *loop, TR_peer *peer, TR_torrent *tr);
void send_handshake(TR_peer *peer, TR_torrent *tr, uint8_t *peer_id);
void on_peer_readable(EV_loop *loop, TR_peer_ctx *context);
void disconnect_peer(EV_loop *loop, TR_peer *peer);
