#ifndef TYPES_H
#define TYPES_H

#include <cstdint>
#include <stddef.h>
#include <stdint.h>

#define MAX_CHAR 512

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

struct BEN_value {
    BEN_type type;
    union {
        int64_t number;
        BEN_string string;
        struct {
            BEN_value **items;
            int count;
        } list;
        BEN_pairs dict;
    };
};



typedef struct {
    char *announce;
    int tier;
} TR_tracker;

typedef struct {
    char *name;
    char *torrent_path;
    char *created_by;
    char *comment;

    int64_t creation_date;

    uint8_t info_hash[20];
    uint64_t total_size;

    TR_tracker *trackers;
    int trackers_length;

    TR_file *files;
    int64_t piecies_length;

} TR_info;


#endif 
