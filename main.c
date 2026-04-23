#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "types/types.h"
#include "utils/file_interactions.h"
#include "parser/parser.h"

int main(int argc, char **argv)
{
    file_content_buffer buffer;
    BEN_value *top_dict;
    BEN_parser parser;
    TR_info *info = malloc(sizeof(TR_info));
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


    // printf("Trying to parse into TR_info"); //debug
    init_BEN_parser(buffer, &parser);
    top_dict = parse_file_content_buffer(&parser);

    if (top_dict->dict.count == 0)
    {
        fprintf(stderr, "ERROR: file couldnt be parsed\n");
        exit(EXIT_FAILURE);
    }

    // printf("Trying to parse into TR_info"); //debug
    BEN_pairs_to_TR_info(&top_dict->dict, info);

    get_info_hash(&parser, hash);
    // printf("Name: %s\n", info->name);
    // printf("Created by: %s\n", info->created_by);
    // printf("Comment: %s\n", info->comment);
    // printf("Piece length: %ld\n", info->piece_length);
    // printf("Announce: %s\n", info->trackers[0].announce);
    // printf("Filepath: %s\n", info->files[0].path);
    printf("Hash: ");
    for(int i = 0; i < 20; i++) {
        printf("%02x", hash[i]);
    }
    printf("\n");

    printf("\n");
    printf("Parsing successful\n");

}
