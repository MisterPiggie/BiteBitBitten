#include "parser.h"
#include "../SHA1/SHA1.h"
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
    if (b_key->length != strlen(key))
            return false;
    if (strncmp((char *)b_key->data, key, b_key->length) == 0) //CHANGED MEMORY LEAK
        return true;
    return false;
}

BEN_value *get_BEN_value_by_key(const BEN_pairs *pairs, const char *key)
{
    int i;
    for (i = 0; i < pairs->count; i++)
    {
        if (BEN_string_equals(&pairs->BEN_pairs[i].key, key))
            return pairs->BEN_pairs[i].value;
    }
    return NULL;
}


void BEN_pairs_to_TR_info(const BEN_pairs *pairs, TR_info *info)
{
    BEN_value *temp_b_value;

    parse_BEN_announce_to_TR_info(pairs, info);

    if ((temp_b_value = get_BEN_value_by_key(pairs, "info")))
    {
        parse_BEN_info_to_TR_info(&temp_b_value->dict, info);
    }

    if ((temp_b_value = get_BEN_value_by_key(pairs, "created by")))
    {
        info->created_by= BEN_string_to_C_string(&temp_b_value->string);
    }

    if ((temp_b_value = get_BEN_value_by_key(pairs, "comment")))
    {
        info->comment = BEN_string_to_C_string(&temp_b_value->string);
    }

    if ((temp_b_value = get_BEN_value_by_key(pairs, "creation date")))
    {
        info->creation_date = temp_b_value->number;
    }
}


void parse_BEN_info_to_TR_info(const BEN_pairs *b_info, TR_info *info)
{
    BEN_value *temp_b_value;
    temp_b_value = get_BEN_value_by_key(b_info, "name");
    info->name = BEN_string_to_C_string(&temp_b_value->string);

    temp_b_value = get_BEN_value_by_key(b_info, "piece length");
    info->piece_length = temp_b_value->number;

    temp_b_value = get_BEN_value_by_key(b_info, "pieces");
    parse_BEN_pieces_to_TR_info(&temp_b_value->string, info);

    temp_b_value = get_BEN_value_by_key(b_info, "files");
    if (temp_b_value != NULL)
    {
        parse_BEN_multifile_list_to_TR_info(&temp_b_value->list, info);
        return;
    } else if ((temp_b_value = get_BEN_value_by_key(b_info, "length")) != NULL)
    {
        info->files = malloc(sizeof(TR_file));
        info->files->length = temp_b_value->number;
        info->files->path = info->name;
        return;
    }
}

void parse_BEN_announce_to_TR_info(const BEN_pairs *pairs, TR_info *info)
{
    BEN_value *announce_list;
    int i, j, temp_trackers_length = 0;

    info->trackers_length = 0;

    if ((announce_list = get_BEN_value_by_key(pairs, "announce-list")))
    {
        for (i = 0; i < announce_list->list.count; i++)
        {
            {
                info->trackers_length += announce_list->list.items[i]->list.count;
            }
        }

        info->trackers = malloc(sizeof(TR_tracker) * info->trackers_length);
        if (info->trackers == NULL)
            return;
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
        return;
    } else if ((announce_list = get_BEN_value_by_key(pairs, "announce")))
    {   
        info->trackers_length++;
        info->trackers[temp_trackers_length].announce = BEN_string_to_C_string(&announce_list->string);
        info->trackers[temp_trackers_length].tier = 0;
        temp_trackers_length++;
    }


}


void parse_BEN_pieces_to_TR_info(BEN_string *b_str, TR_info *info)
{
    info->pieces = malloc(b_str->length * sizeof(uint8_t));
    memcpy(info->pieces, b_str->data, b_str->length);

    info->pieces_string_length = b_str->length;
    return;
}

void parse_BEN_multifile_list_to_TR_info(BEN_list *b_list, TR_info *info)
{
    int i;

    info->files_count = b_list->count;
    info->files = malloc(sizeof(TR_info) * info->files_count);

    for (i = 0; i<b_list->count; i++)
    {
        info->files[i].length = get_BEN_value_by_key(&b_list->items[i]->dict, "length")->number;
        info->files[i].path = parse_BEN_list_to_path_C_string(
                &get_BEN_value_by_key(&b_list->items[i]->dict, "path")->list
                );
    }
}

char *parse_BEN_list_to_path_C_string(BEN_list *b_list)
{
    int path_length = 0, i = 0;
    char *path_str;
    char *filename;
    for (i = 0; i < b_list->count; i++)
    {
        path_length += b_list->items[i]->string.length;
    }
    path_length += b_list->count + 1; //for "/" symbol and NULL terminator 
    
    path_str = malloc(sizeof(char) * path_length);
    path_str[0] = '\0';

    for (i = 0; i < b_list->count; i++)
    {
        filename = BEN_string_to_C_string(&b_list->items[i]->string);
        strcat(path_str, "/");
        strcat(path_str, filename); //NEED TO CHANGE
        free(filename);
    }

    return path_str;
}

void fill_in_calculated_field_in_TR_info(TR_info *info)
{
    uint64_t total_size = 0;
    int i;

    for (i = 0; i < info->files_count; i++)
    {
        info->files[i].offset = total_size;

        info->files[i].first_piece = info->files[i].offset / info->piece_length;
        info->files[i].last_piece = 
            (info->files[i].offset + info->files[i].length - 1) / info->piece_length;

        total_size += info->files[i].length;
    }

    info->total_size = total_size;
}
