#include <stddef.h>
#include <stdint.h>
#include "../types/types.h"


void init_BEN_parser(file_content_buffer buffer, BEN_parser *parser);
BEN_value *parse_file_content_buffer(BEN_parser *parser);

unsigned char peek(BEN_parser *parser);
unsigned char consume(BEN_parser *parser);

BEN_string parse_raw_string(BEN_parser *parser);

BEN_value *parse_dict(BEN_parser *parser);
BEN_value *parse_string(BEN_parser *parser);
BEN_value *parse_num(BEN_parser *parser);
BEN_value *parse_list(BEN_parser *parser);

BEN_value *parse_value(BEN_parser *parser);

void get_info_value_offset(BEN_parser *parser, int *begining, int *end);
void skip_value(BEN_parser *parser);
void get_info_hash(BEN_parser *parser, unsigned char info_hash[20]);


