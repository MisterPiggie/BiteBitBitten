#ifndef TYPES_H
#define TYPES_H

#include <stddef.h>
#include <stdint.h>


typedef struct {
    unsigned char *data;
    size_t size;
} file_content_buffer;

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


typedef struct {
    BEN_string key;
    BEN_value *value;
} BEN_pair;

typedef struct {
    BEN_pair *BEN_pairs;
    int count;
} BEN_pairs;

typedef struct {
    file_content_buffer buffer;
    int cursor;
} BEN_parser;

typedef struct {
    BEN_value **items;
    int count;
} BEN_list;


struct BEN_value {
    BEN_type type;
    union {
        int64_t number;
        BEN_string string;
        BEN_list list;
        BEN_pairs dict;
    };
};



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
    int trackers_length;

    TR_file *files;
    int files_count;
    int64_t piece_length;

} TR_info;


#endif 
