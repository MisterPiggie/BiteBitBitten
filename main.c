#include <stddef.h>
#include <openssl/sha.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include "utils/file_interactions.h"
#include "parser/parser.h"
#include "SHA1/SHA1.h"

int main(int argc, char **argv)
{
    file_content_buffer buffer;
    bencode_value *top_dict;
    bencode_parser parser;
    uint8_t hash[20];
    int begining, end;

    if (argc != 2) 
    {
        fprintf(stderr, "ERROR: no file provided\n");
        exit(EXIT_FAILURE);
    }
    
    
    buffer = read_bencoded_file(argv[1]);
    if (buffer.size == 0)
    {
        fprintf(stderr, "ERROR: file couldnt be open\n");
        exit(EXIT_FAILURE);
    }


    init_bencode_parser(buffer, &parser);
    top_dict = parse_file_content_buffer(&parser);

    if (top_dict->dict.count == 0)
    {
        fprintf(stderr, "ERROR: file couldnt be parsed\n");
        exit(EXIT_FAILURE);
    }

    get_info_value_offset(&parser, &begining, &end);
    printf("Beginning %d\nEnd %d\n", begining, end);
    SHA1(parser.buffer.data + begining, end - begining, hash);

    printf("Hash is equal to %s", hash);
    for (int i = 0; i < 20; i++)
    {
        printf("%02x", hash[i]);
    }

    printf("\n");
    printf("Parsing successful\n");

}
