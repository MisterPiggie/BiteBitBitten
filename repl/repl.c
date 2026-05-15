#include <stdio.h>
#include <stdlib.h>
#include "../types/types.h"
#include "../files/file_interactions.h"
#include "../parser/parser.h"

void CMD_add(int argc, char **argv)
{
    file_content_buffer *buffer;
    BEN_parser          *parser;

    if (argc < 2)
    {
        puts("USAGE: add <.torrent filepath>");
        return;
    }

    buffer = read_BEN_file(argv[1]);
    if (!buffer)
        return;
    
    parser = init_BEN_parser(buffer);
    free(buffer->data);
    free(buffer);
    

}
void CMD_print(int argc, char **argv);
void CMD_delete(int argc, char **argv);
void CMD_connect(int argc, char **argv);
void CMD_help(int argc, char **argv);
