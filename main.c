#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include "utils/file_interactions.h"
#include "parser/parser.h"

int main(int argc, char **argv)
{
    file_content_buffer buffer;
    bencode_value *top_dict;
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
    top_dict = parse_file_content_buffer(buffer);
    if (top_dict->dict.count == 0)
    {
        fprintf(stderr, "ERROR: file couldnt be parsed\n");
        exit(EXIT_FAILURE);
    }
    printf("Parsing successful\n");

}
