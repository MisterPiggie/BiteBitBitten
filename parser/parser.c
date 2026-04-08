#include "parser.h"
#include <stdlib.h>

bencode_pairs parse_file_content_buffer(file_content_buffer buffer)
{
    bencode_pairs pairs;
    pairs.bencode_pairs = NULL;
    pairs.count = 0;

    bencode_parser parser;
    parser.buffer = buffer;
    parser.cursor = 0;

    parse_dict(&parser, &pairs);

    free(parser.buffer.data);

    return pairs;
}


void parse_dict(bencode_parser *parser, bencode_pairs *pairs)
{

}
