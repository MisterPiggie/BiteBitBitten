#include <stddef.h>
#include <stdint.h>
#include "../types/types.h"

typedef enum {
    BENCODE_LIST,
    BENCODE_NUMBER,
    BENCODE_STRING,
    BENCODE_DICT,
} bencode_type;

typedef struct bencode_value bencode_value;


typedef struct {
    char *key;
    bencode_value *value;
} bencode_pair;

typedef struct {
    bencode_pair *bencode_pairs;
    int count;
} bencode_pairs;

typedef struct {
    char *data;
    size_t length;
} string;

typedef struct {
    bencode_pairs pairs;
    file_content_buffer buffer;
    int cursor;
} bencode_cursor;

struct bencode_value {
    bencode_type type;
    union {
        int64_t number;
        string string;
        struct {
            bencode_value **items;
            int count;
        } list;
        bencode_pairs dict;
    };
};




bencode_pairs parse_file_content_buffer(file_content_buffer buffer);
