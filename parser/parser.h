#include <stdint.h>
#include "../types/types.h"

typedef enum {
    BENCODE_LIST,
    BENCODE_NUMBER,
    BENCODE_STRING,
} bencode_type;

typedef struct bencode_value bencode_value;


typedef struct {
    char *key;
    bencode_value *value;
} bencode_pair;

struct bencode_value {
    bencode_type type;
    union {
        int64_t number;
        char *string;
        struct {
            bencode_value **items;
            int count;
        } list;
    };
};


typedef struct {
    bencode_pair *bencode_pairs;
    int size;
} bencode_pairs_list;

bencode_pairs_list parse_file_content_buffer(file_content_buffer buffer);
