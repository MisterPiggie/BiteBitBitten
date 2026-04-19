#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
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

void BEN_pairs_to_TR_info(const BEN_pairs *pairs, TR_info *info);
char *BEN_string_to_C_string(const BEN_string *b_string);
void BEN_file_into_TR_file(const BEN_pairs *pairs, TR_file);
BEN_value *get_BEN_value_by_key(const BEN_pairs *pairs, const char *key); 

bool BEN_string_equals(BEN_string *b_key, const char *key);
