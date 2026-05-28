#ifndef TYPES_H
#define TYPES_H

#include <stddef.h>
#include <time.h>
#include <stdint.h>
#include "../arena/arena.h"


#define PEER_AM_CHOKING (1 << 0)
#define PEER_AM_INTERESTED (1 << 1)
#define PEER_CHOKING_US (1 << 2)
#define PEER_INTERESTED_IN_US (1 << 3)

typedef enum {
    NOT_CONNECTED,
    CONNECTED,
    BANNED,
}

typedef enum {
    NEEDS_ANNOUNCE,
    ANNOUNCING,
    ANNOUNCED,
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
    uint64_t total_size;

    TR_tracker *trackers;
    int trackers_count;

    TR_file *files;
    int files_count;
    int64_t piece_length;

} TR_info;


typedef struct {
    uint64_t connection_id;
    uint32_t action;
    uint32_t transction_id;
    uint8_t  *hash_info;
    uint64_t downloaded;
    uint64_t left;
    uint64_t uploaded;
    uint32_t event;
    uint32_t ip;
    uint32_t key;
    int32_t  num_want;
    uint16_t port;
} UDP_request;



typedef struct {
    char     *schema;
    char     *host;
    char     *path;
    uint16_t port;

    uint64_t connection_id;

    UDP_request *reqs;
    int         reqs_count;

    uint32_t    key;
} NET_tracker;

typedef struct
}
    uint32_t       ip;
    uint8_t        port;  

    int            sock;

    TR_peer_state  peer_state;
    uint8_t        bitmask_state;

    uint8_t        *bitfield;
    int            bitfield_len;

    time_t         last_tried;
    int            tries_count;
}

typedef struct 
{
    TR_peer *peer_pool[200];
    int     pool_count;

    TR_peer *peers[50];
    int     peers_count; 
}


typedef struct {
    Arena       arena;

    TR_info         *info;
    NET_tracker     *tracks;
    int             tracker_count;
    int             active_tracker_idx;
    char            *download_path;
    char            *torrent_file_path;

    int             tcp_socket;
    uint64_t        downloaded;
    uint64_t        uploaded;

    TR_track_state  track_state;
    time_t          next_announce;

    TR_swarm        *swarm;
} TR_torrent;

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

#endif 
