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


    init_BEN_parser(buffer, &parser);
    top_dict = parse_file_content_buffer(&parser);

    if (top_dict->dict.count == 0)
    {
        fprintf(stderr, "ERROR: file couldnt be parsed\n");
        exit(EXIT_FAILURE);
    }



    BEN_pairs_to_TR_info(&top_dict->dict, info);
    printf("Name: %s\n", info->name);
    printf("Created by: %s\n", info->created_by);
    printf("Creation date: %ld\n", info->creation_date);
    printf("Trackers: \n");
    for (int i = 0; i < info->trackers_length; i++)
    {
        printf("    %s\n", info->trackers->announce);
    }
    printf("Amount of files: %d\n", info->files_count);
    for (int i = 0; i < info->files_count; i++)
    {
        printf("    Filepath: %s\n", info->files[i].path);
    }

    get_info_hash(&parser, info->info_hash);
    printf("Hash: ");
    for(int i = 0; i < 20; i++) {
        printf("%02x", info->info_hash[i]);
    }
    printf("\n");

    printf("\n");
    printf("Parsing successful\n");

}
