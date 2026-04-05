#include <stdint.h>

typedef enum {
    BENCODE_LIST,
    BENCODE_INT,
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
        } array;
    };
};
