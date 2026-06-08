#ifndef TYPES_H
#define TYPES_H

#include <stddef.h>
#include <time.h>
#include <stdint.h>
#include <stdbool.h>
#include <pthread.h>
#include "../arena/arena.h"


#define PEER_AM_CHOKING (1 << 0)
#define PEER_AM_INTERESTED (1 << 1)
#define PEER_CHOKING_US (1 << 2)
#define PEER_INTERESTED_IN_US (1 << 3)

typedef enum 
{
    TORRENT_DOWNLOADING,
    TORRENT_NEEDS_CHECK,
    TORRENT_CHECKING,
    TORRENT_SEEDING,
    TORRENT_PAUSED,
    TORRENT_FAILED,
} TR_state;

typedef enum {
    NOT_CONNECTED,
    CONNECTING,
    CONNECTED,
    HANDSHAKING,
    BANNED,
} TR_peer_state;

typedef enum {
    TRACKER_NOT_RESOLVED,
    TRACKER_RESOLVING,
    TRACKER_IDLE,
    TRACKER_ANNOUNCING,
    TRACKER_ALIVE,
    TRACKER_FAILED,
} TR_track_state;

//FILE structs
typedef struct {
    unsigned char *data;
    size_t size;
} file_content_buffer;


//BEN structs
typedef enum {
    BENCODE_LIST,
    BENCODE_NUMBER,
    BENCODE_STRING,
    BENCODE_DICT,
} BEN_type;

typedef struct BEN_value BEN_value;


typedef struct {
    unsigned char *data;
    size_t length;
} BEN_string;


typedef struct BEN_pair BEN_pair;
struct BEN_pair{
    BEN_string key;
    BEN_value *value;
    BEN_pair  *next;
};


typedef struct {
    file_content_buffer buffer;
    int cursor;
} BEN_parser;

typedef struct BEN_list BEN_list;
struct BEN_list{
    BEN_value *value;
    BEN_list *next;
};


struct BEN_value {
    BEN_type type;
    union {
        int64_t number;
        BEN_string string;
        BEN_list *list;
        BEN_pair *dict;
    };
};



//Torrent structs
typedef struct {
    char *announce;
    int tier;
} TR_tracker;

typedef struct {
    char *path;

    uint64_t length;
    uint64_t offset;

    uint32_t first_piece;
    uint32_t last_piece;
} TR_file;

typedef struct {
    char *name;
    char *torrent_path;
    char *created_by;
    char *comment;

    int64_t creation_date;

    uint8_t info_hash[20];
    uint8_t *pieces;
    size_t pieces_string_length;
    uint64_t pieces_count;
    uint64_t total_size;

    TR_tracker *trackers;
    int trackers_count;

    TR_file *files;
    int files_count;
    int64_t piece_length;

} TR_info;





typedef struct {
    char      *schema;
    char      *host;
    char      *path;
    uint16_t  port;

    uint64_t  connection_id;
    TR_track_state state;

    time_t    last_announce;
    time_t    announce_sent_at;

    int       interval;

    uint32_t  ip;
} NET_tracker;



typedef struct TR_peer    TR_peer;
typedef struct TR_torrent TR_torrent;

typedef struct 
{
    TR_peer *peer;
    TR_torrent *tr;
}TR_peer_ctx;

struct TR_peer
{
    uint32_t       ip;
    uint16_t       port;  

    int            sock;

    TR_peer_state  peer_state;
    uint8_t        bitmask_state;

    uint8_t        *bitfield;
    int            bitfield_len;

    time_t         last_tried;
    int            failed_tries;

    TR_peer_ctx    context;
}; 


typedef struct 
{
    TR_peer *peer_pool[200];
    int     pool_count;

    TR_peer *peers[50];
    int     peers_count; 
} TR_swarm;


struct TR_torrent{
    Arena       arena;
    TR_state    state;

    TR_info         *info;
    NET_tracker     *tracks;
    int             tracker_count;
    int             active_tracker_idx;
    char            *download_path;
    char            *torrent_file_path;

    uint64_t        downloaded;
    uint64_t        uploaded;

    uint8_t         *bitfield;
    uint32_t        bitfield_length;

    TR_swarm        *swarm;
};


typedef struct {
    uint32_t    transaction_id;
    time_t      sent_at;

    TR_torrent  *torrent;
    NET_tracker *tracker;
} UDP_request;

typedef struct {
    
    char *download_folder_path;
    char *resume_dir_path;
    char *torrent_dir_path;
    char *config_dir_path;
    char *config_file_path;

    uint8_t      peer_id[20];
    TR_torrent   **torrents;
    int          torrents_count;
    Arena        *main_arena;
} CL_session;


typedef void (*task_fn)(void *arg);

typedef struct
{
    task_fn fn;
    void    *arg;
} CL_task;

typedef struct
{
    pthread_t   threads[4];
    CL_task     queue[64];

    int         head;
    int         tail;
    int         count;

    pthread_mutex_t lock;
    pthread_cond_t  cond;

    bool stop;
} CL_threadpool;

typedef struct
{
    CL_session    *session;
    CL_threadpool *pool;

    int         epollfd;
    int         client_timerfd;
    int         peer_timerfd;
    int         notify_pipe[2];

    UDP_request udp_requests[128];
    int         udp_requests_count;

    int         udp_socket;
} EV_loop;


#endif 
