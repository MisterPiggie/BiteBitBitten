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
} bencode_type;

typedef struct bencode_value bencode_value;


typedef struct {
    unsigned char *data;
    size_t length;
} bencode_string;


typedef struct {
    bencode_string key;
    bencode_value *value;
} bencode_pair;

typedef struct {
    bencode_pair *bencode_pairs;
    int count;
} bencode_pairs;

typedef struct {
    file_content_buffer buffer;
    int cursor;
} bencode_parser;

struct bencode_value {
    bencode_type type;
    union {
        int64_t number;
        bencode_string string;
        struct {
            bencode_value **items;
            int count;
        } list;
        bencode_pairs dict;
    };
};



#endif 
