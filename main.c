#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "utils/file_interactions.h"
#include "parser/parser.h"

int main(int argc, char **argv)
{
    file_content_buffer buffer;
    BEN_value *top_dict;
    BEN_parser parser;
    uint8_t hash[20];
    memset(hash, 0, sizeof(hash));
    if (argc != 2) 
    {
        fprintf(stderr, "ERROR: no file provided\n");
        exit(EXIT_FAILURE);
    }
    
    
    buffer = read_BEN_file(argv[1]);
    if (buffer.size == 0)
    {
        fprintf(stderr, "ERROR: file couldnt be open\n");
        exit(EXIT_FAILURE);
    }


    init_BEN_parser(buffer, &parser);
    top_dict = parse_file_content_buffer(&parser);

    if (top_dict->dict.count == 0)
    {
        fprintf(stderr, "ERROR: file couldnt be parsed\n");
        exit(EXIT_FAILURE);
    }

    get_info_hash(&parser, hash);

    printf("Hash: ");
    for(int i = 0; i < 20; i++) {
        printf("%02x", hash[i]);
    }
    printf("\n");

    printf("\n");
    printf("Parsing successful\n");

}
