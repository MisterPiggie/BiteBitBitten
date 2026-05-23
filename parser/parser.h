
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include "../arena/arena.h"
#include "../types/types.h"


//parser init
BEN_parser *init_BEN_parser(Arena *arena, file_content_buffer *buffer);
void init_reserved_table(void);

//parser helper funcs
unsigned char peek(BEN_parser *parser);
unsigned char consume(BEN_parser *parser);

BEN_string parse_raw_string(BEN_parser *parser);

//parsing action
BEN_value *parse_dict(Arena *arena, BEN_parser *parser);
BEN_value *parse_string(Arena *arena, BEN_parser *parser);
BEN_value *parse_num(Arena *arena, BEN_parser *parser);
BEN_value *parse_list(Arena *arena, BEN_parser *parser);

BEN_value *parse_value(Arena *arena, BEN_parser *parser);

//SHA1 helper funcs
void get_info_value_offset(BEN_parser *parser, int *begining, int *end);
void skip_value(BEN_parser *parser);
void get_info_hash(BEN_parser *parser, unsigned char info_hash[20]);

//parsing inside struct 
TR_info *BEN_pairs_to_TR_info(Arena *arena, const BEN_pair *pair);
void fill_in_calculated_field_in_TR_info(TR_info *info);

//complex operations inside BEN_pairs_to_TR_info
bool parse_BEN_info_to_TR_info(Arena *arena, const BEN_pair *b_info, TR_info *info);
bool parse_BEN_announce_to_TR_info(Arena *arena, const BEN_pair *pairs, TR_info *info);

//BEN to TR helper funcs
BEN_value *get_BEN_value_by_key(const BEN_pair *pairs, const char *key); 
char *BEN_string_to_C_string(Arena *arena, const BEN_string *b_string);
void parse_BEN_pieces_to_TR_info(Arena *arena, BEN_string *b_str, TR_info *info);
bool BEN_string_equals(const BEN_string *b_key, const char *key);
void parse_BEN_multifile_list_to_TR_info(Arena *arena, BEN_list *b_list, TR_info *info);
char *parse_BEN_list_to_path_C_string(Arena *arena, BEN_list *b_list);
