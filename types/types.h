#ifndef TYPES_H
#define TYPES_H

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

typedef enum {
    MULTI_FILE,
    SINGLE_FILE,
} TR_info_type;


typedef struct {
    char **path;
    int path_length;
    uint64_t length;
} TR_file;

typedef struct {
    char *name;
    TR_info_type type;
    int64_t piece_length; 
    uint8_t *pieces;
    TR_file *files;
    int TR_file_length;
    int64_t length;
} TR_info;

typedef struct {
    char announce[MAX_CHAR];
    char *announce_list[MAX_CHAR];
    int announce_list_length;
    char created_by[MAX_CHAR];
    int64_t creation_date;
    TR_info info;
} TR_file_data;


#endif 
