#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include "../types/types.h"


//parser init
void init_BEN_parser(file_content_buffer buffer, BEN_parser *parser);
BEN_value *parse_file_content_buffer(BEN_parser *parser);

//parser helper funcs
unsigned char peek(BEN_parser *parser);
unsigned char consume(BEN_parser *parser);

BEN_string parse_raw_string(BEN_parser *parser);

//parsing action
BEN_value *parse_dict(BEN_parser *parser);
BEN_value *parse_string(BEN_parser *parser);
BEN_value *parse_num(BEN_parser *parser);
BEN_value *parse_list(BEN_parser *parser);

BEN_value *parse_value(BEN_parser *parser);

//SHA1 helper funcs
void get_info_value_offset(BEN_parser *parser, int *begining, int *end);
void skip_value(BEN_parser *parser);
void get_info_hash(BEN_parser *parser, unsigned char info_hash[20]);

//parsing inside struct 
void BEN_pairs_to_TR_info(const BEN_pairs *pairs, TR_info *info);
void fill_in_calculated_field_in_TR_info(TR_info *info);

//complex operations inside BEN_pairs_to_TR_info
void parse_BEN_info_to_TR_info(const BEN_pairs *b_info, TR_info *info);
void parse_BEN_announce_to_TR_info(const BEN_pairs *pairs, TR_info *info);

//BEN to TR helper funcs
BEN_value *get_BEN_value_by_key(const BEN_pairs *pairs, const char *key); 
char *BEN_string_to_C_string(const BEN_string *b_string);
void parse_BEN_pieces_to_TR_info(BEN_string *b_str, TR_info *info);
bool BEN_string_equals(BEN_string *b_key, const char *key);
void parse_BEN_multifile_list_to_TR_info(BEN_list *b_list, TR_info *info);
char *parse_BEN_list_to_path_C_string(BEN_list *b_list);
