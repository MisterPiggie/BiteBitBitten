#include "parser.h"
#include "../SHA1/SHA1.h"
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>


void init_BEN_parser(file_content_buffer buffer, BEN_parser *parser)
{
    parser->buffer = buffer;
    parser->cursor = 0;
}

BEN_value *parse_file_content_buffer(BEN_parser *parser)
{
    BEN_value *ret_dict;

    ret_dict = parse_dict(parser);

    return ret_dict;
}


BEN_value *parse_dict(BEN_parser *parser)
{
    BEN_string key;
    BEN_value *value; 

    BEN_value *return_dict= malloc(sizeof(BEN_value));

    return_dict->type = BENCODE_DICT;
    return_dict->dict.BEN_pairs = NULL;
    return_dict->dict.count = 0;

    consume(parser);

    while(peek(parser) != 'e')
    {
        key = parse_raw_string(parser);
        value = parse_value(parser);

        printf("%.*s\n", (int) key.length, key.data); //debug

        return_dict->dict.BEN_pairs = realloc(
                return_dict->dict.BEN_pairs,
                sizeof(BEN_pair) * (return_dict->dict.count + 1)
               );
        return_dict->dict.BEN_pairs[return_dict->dict.count].key = key;
        return_dict->dict.BEN_pairs[return_dict->dict.count].value = value;
        return_dict->dict.count++;
    }
    consume(parser); // consume final e
    return return_dict;
}

BEN_value *parse_string(BEN_parser *parser)
{
    BEN_value *return_val = malloc(sizeof(BEN_value));
    return_val->type = BENCODE_STRING;
    return_val->string = parse_raw_string(parser);

    return return_val;
}

BEN_value *parse_num(BEN_parser *parser)
{
    int64_t num; 
    char *end;
    BEN_value *return_val = malloc(sizeof(BEN_value));

    consume(parser); //consume i
    num = strtol((char *) parser->buffer.data + parser->cursor, &end, 10);
    
    return_val->type = BENCODE_NUMBER;
    return_val->number = num;

    parser->cursor = end - (char *) parser->buffer.data; //might be problem with cast

    consume(parser); //consume e
    
    return return_val;
}

BEN_value *parse_list(BEN_parser *parser)
{
    BEN_value *return_val = malloc(sizeof(BEN_value));
    return_val->type = BENCODE_LIST;
    return_val->list.items = NULL;
    return_val->list.count = 0;

    consume(parser); //consume l

    while(peek(parser) != 'e')
    {
        return_val->list.items = realloc(
                return_val->list.items, 
                sizeof(BEN_value *) * (return_val->list.count + 1)
        );

        return_val->list.items[return_val->list.count++] = parse_value(parser);
    }

    consume(parser); //consume e

    return return_val;
}

unsigned char peek(BEN_parser *parser)
{
    return parser->buffer.data[parser->cursor];
}

unsigned char consume(BEN_parser *parser)
{
     return parser->buffer.data[parser->cursor++]; // might be problem
}


BEN_string parse_raw_string(BEN_parser *parser)
{
    char *delimeter;
    BEN_string raw_str;

    raw_str.length = strtol((char *) parser->buffer.data + parser->cursor, &delimeter, 10); //might be problem with cast
    parser->cursor = delimeter - (char *) parser->buffer.data; //might be problem with cast
    consume(parser);

    raw_str.data = parser->buffer.data + parser->cursor;
    parser->cursor += raw_str.length;
    return raw_str;
}

BEN_value *parse_value(BEN_parser *parser)
{
    switch (peek(parser))
    {
        case 'i': return parse_num(parser);
        case 'l': return parse_list(parser);
        case 'd': return parse_dict(parser);
        default: return parse_string(parser);
    }
}


void get_info_value_offset(BEN_parser *parser, int *begining, int *end)
{
    BEN_string key; 

    parser->cursor = 0;
    consume(parser);

    while(peek(parser) != 'e')
    {
        key = parse_raw_string(parser);

        if ((key.length == 4) && (strncmp((char *)key.data, "info", key.length) == 0))
        {
            *begining = parser->cursor;
            skip_value(parser);
            *end = parser->cursor;
            return;
        }

        skip_value(parser);
    }
}

void skip_value(BEN_parser *parser)
{
    switch (peek(parser)) {
        case 'd': case 'l':
            consume(parser);
            while (peek(parser) != 'e')
                skip_value(parser);
            consume(parser);
            break;
        case 'i':
            while (consume(parser) != 'e')
                ;
            break;
        case '0': case '1': case '2': case '3': case '4': 
        case '5': case '6': case '7': case '8': case '9': 
            parse_raw_string(parser); //skip string value
            break;
    }
}

void get_info_hash(BEN_parser *parser, unsigned char info_hash[20])
{
    int begining, end;
    get_info_value_offset(parser, &begining, &end);
    SHA1_hash(parser->buffer.data + begining, end - begining, info_hash);
}

char *BEN_string_to_C_string(const BEN_string *b_string)
{
    return strndup((char *)b_string->data, b_string->length);
}

bool BEN_string_equals(BEN_string *b_key, const char *key)
{
   if (strcmp(BEN_string_to_C_string(b_key), key) == 0)
       return true;
   return false;
}

BEN_value *get_BEN_value_by_key(const BEN_pairs *pairs, const char *key)
{
    int i;
    for (i = 0; i < pairs->count; i++)
    {
        if (BEN_string_equals(&pairs->BEN_pairs->key, key))
            return pairs->BEN_pairs->value;
    }
    return NULL;
}


void BEN_pairs_to_TR_info(const BEN_pairs *pairs, TR_info *info)
{
    BEN_value *temp_b_value;
    if ((temp_b_value = get_BEN_value_by_key(pairs, "info")))
        parse_BEN_info_to_TR_info(&temp_b_value->dict, info);

    if ((temp_b_value = get_BEN_value_by_key(pairs, "announce")))
        parse_BEN_announce_to_TR_info(pairs, info);

    if ((temp_b_value = get_BEN_value_by_key(pairs, "created by")))
        info->created_by= BEN_string_to_C_string(&temp_b_value->string);

    if ((temp_b_value = get_BEN_value_by_key(pairs, "comment")))
        info->comment = BEN_string_to_C_string(&temp_b_value->string);

    if ((temp_b_value = get_BEN_value_by_key(pairs, "creation date")))
        info->creation_date = temp_b_value->number;
}


void parse_BEN_info_to_TR_info(const BEN_pairs *b_info, TR_info *info);

void parse_BEN_announce_to_TR_info(const BEN_pairs *pairs, TR_info *info)
{
    BEN_value *announce;
    BEN_value *announce_list;
    int i, j, temp_trackers_length = 0;

    info->trackers_length = 0;

    announce = get_BEN_value_by_key(pairs, "announce");
    if (announce)
    {   
        info->trackers_length++;
    }

    announce_list = get_BEN_value_by_key(pairs, "announce-list");
    if (announce_list)
    {
        for (i = 0; i < announce_list->list.count; i++)
        {
            if (announce_list->list.items[i]->type == BENCODE_LIST)
                info->trackers_length += announce_list->list.items[i]->list.count;
        }
    }

    if (temp_trackers_length == 0)
        return;
    info->trackers = malloc(sizeof(TR_tracker) * info->trackers_length);
    if (info->trackers == NULL)
        return;

    if (announce)
    {
        info->trackers[temp_trackers_length].announce = BEN_string_to_C_string(&announce->string);
        info->trackers[temp_trackers_length].tier = 0;
        temp_trackers_length++;
    }

    if (announce_list)
    {
        for (i = 0; i < announce_list->list.count; i++)
        {

            if (announce_list->list.items[i]->type == BENCODE_LIST)
                for (j = 0; j < announce_list->list.items[i]->list.count; j++)
                {
                    info->trackers[temp_trackers_length].announce = 
                        BEN_string_to_C_string(
                                &announce_list->list.items[i]->list.items[j]->string
                                );

                    info->trackers[temp_trackers_length].tier = i;
                    temp_trackers_length++;
                }
        }
    }
}
