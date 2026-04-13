#include <stddef.h>
#include <stdint.h>
#include "../types/types.h"


bencode_value *parse_file_content_buffer(file_content_buffer buffer);

unsigned char peek(bencode_parser *parser);
unsigned char consume(bencode_parser *parser);

bencode_string parse_raw_string(bencode_parser *parser);

bencode_value *parse_dict(bencode_parser *parser);
bencode_value *parse_string(bencode_parser *parser);
bencode_value *parse_num(bencode_parser *parser);
bencode_value *parse_list(bencode_parser *parser);

bencode_value *parse_value(bencode_parser *parser);

void get_info_value_offset(bencode_parser *parser, int *begining, int *end);
