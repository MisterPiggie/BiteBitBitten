#include "parser.h"
#include "../SHA1/SHA1.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "../arena/arena.h"

bool reserved_table[256] = {
    ['<'] = true, ['>'] = true, [':'] = true, 
    ['"'] = true, ['/'] = true, ['\\'] = true, 
    ['|'] = true, ['?'] = true, ['*'] = true
}


BEN_parser *init_BEN_parser(Arena *arena, file_content_buffer *buffer)
{
    BEN_parser *parser = arena_push_struct(arena, BEN_parser);
    parser->buffer = *buffer;
    parser->cursor = 0;

    return parser;
}

void init_reserved_table(void)
{
    for (int i = 0; i < 32; i++)
        reserved_table[i] = true;
}


BEN_value *parse_dict(Arena *arena, BEN_parser *parser)
{
    BEN_pair *tmp_pair;
    BEN_pair *last = NULL;

    BEN_value *return_dict = arena_push_struct(arena, BEN_value);

    return_dict->type = BENCODE_DICT;
    return_dict->dict = NULL;

    if (consume(parser) != 'd')
    {
        printf("ERROR: malformed torrent file\n");
        return NULL;
    }

    while(peek(parser) != 'e')
    {
        tmp_pair        = arena_push_struct(arena, BEN_pair);
        tmp_pair->key   = parse_raw_string(parser);
        tmp_pair->value = parse_value(arena, parser);
        tmp_pair->next  = NULL;
        if (!tmp_pair->value)
            return NULL;

        if (last)
            last->next        = tmp_pair;
        else
            return_dict->dict = tmp_pair;

        last = tmp_pair;
    }
    if (consume(parser) != 'e')
    {
        printf("ERROR: malformed torrent file\n");
        return NULL;
    }
    return return_dict;
}

BEN_value *parse_string(Arena *arena, BEN_parser *parser)
{
    BEN_value *return_val = arena_push_struct(arena, BEN_value);

    return_val->type = BENCODE_STRING;
    return_val->string = parse_raw_string(parser);

    return return_val;
}

BEN_value *parse_num(Arena *arena, BEN_parser *parser)
{
    int64_t num; 
    char *end;
    BEN_value *return_val = arena_push_struct(arena, BEN_value);

    consume(parser); //consume i
    num = strtol((char *) parser->buffer.data + parser->cursor, &end, 10);
    
    return_val->type = BENCODE_NUMBER;
    return_val->number = num;

    parser->cursor = end - (char *) parser->buffer.data; //might be problem with cast

    if (consume(parser) != 'e')
    {
        printf("ERROR: malformed torrent file\n");
        return NULL;
    }
    
    return return_val;
}

BEN_value *parse_list(Arena *arena, BEN_parser *parser)
{
    BEN_list *tmp_list;
    BEN_list *last = NULL;

    BEN_value *val;
    BEN_value *return_val = arena_push_struct(arena, BEN_value);
    return_val->type = BENCODE_LIST;
    return_val->list = NULL;

    consume(parser); //consume l

    while(peek(parser) != 'e')
    {
        val = parse_value(arena, parser);
        if (!val)
            return NULL;

        tmp_list = arena_push_struct(arena, BEN_list);
        tmp_list->value = val;
        tmp_list->next = NULL;

        if (last)
            last->next = tmp_list;
        else
            return_val->list = tmp_list;

        last = tmp_list;
    }

    if (consume(parser) != 'e')
    {
        printf("ERROR: malformed torrent file\n");
        return NULL;
    }

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

    raw_str.length = strtol((char *) parser->buffer.data + parser->cursor, &delimeter, 10); 
    parser->cursor = delimeter - (char *) parser->buffer.data; 
    consume(parser);

    raw_str.data = parser->buffer.data + parser->cursor;
    parser->cursor += raw_str.length;
    return raw_str;
}

BEN_value *parse_value(Arena *arena, BEN_parser *parser)
{
    switch (peek(parser))
    {
        case 'i': return parse_num(arena, parser);
        case 'l': return parse_list(arena, parser);
        case 'd': return parse_dict(arena, parser);

        case '0': case '1': case '2': case '3': case '4':
        case '5': case '6': case '7': case '8': case '9': 
                  return parse_string(arena, parser);

        default:      
                  printf("ERROR: malformed .torrent file\n");
                  return NULL;    
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

char *BEN_string_to_C_string(Arena *arena, const BEN_string *b_string)
{
    return arena_push_strn(arena, (char *) b_string->data, b_string->length);
}

bool BEN_string_equals(const BEN_string *b_key, const char *key)
{
    if (b_key->length != strlen(key))
            return false;
    if (strncmp((char *)b_key->data, key, b_key->length) == 0) //CHANGED MEMORY LEAK
        return true;
    return false;
}

BEN_value *get_BEN_value_by_key(const BEN_pair *pair, const char *key)
{
    const BEN_pair *tmp_pair = pair;

    while (tmp_pair != NULL)
    {
        if (BEN_string_equals(&tmp_pair->key, key))
            return tmp_pair->value;
        tmp_pair = tmp_pair->next;
    }

    return NULL;
}


TR_info *BEN_pairs_to_TR_info(Arena *torrent_arena, const BEN_pair *pair)
{
    BEN_value *temp_b_value;

    TR_info *info = arena_push_struct(torrent_arena, TR_info);
     
    if (parse_BEN_announce_to_TR_info(torrent_arena, pair, info) == false)
    {
        arena_destroy(torrent_arena);
        return NULL;
    }
    temp_b_value = get_BEN_value_by_key(pair, "info");
    if(temp_b_value == NULL)
    {
        printf("ERROR: torrent file doesnt have essential fields");
        arena_destroy(torrent_arena);
        return NULL;
    }

    if (parse_BEN_info_to_TR_info(torrent_arena, temp_b_value->dict, info) == false)
    {
        arena_destroy(torrent_arena);
        return NULL;
    }

    if ((temp_b_value = get_BEN_value_by_key(pair, "created by")))
    {
        info->created_by= BEN_string_to_C_string(torrent_arena, &temp_b_value->string);
        if (info->created_by == NULL)
        {
            printf("ERROR: not enough memory to create info struct\n");
            return NULL;
        }
    }

    if ((temp_b_value = get_BEN_value_by_key(pair, "comment")))
    {
        info->comment = BEN_string_to_C_string(torrent_arena, &temp_b_value->string);
        if (info->comment == NULL)
        {
            printf("ERROR: not enough memory to create info struct\n");
            return NULL;
        }
    }

    if ((temp_b_value = get_BEN_value_by_key(pair, "creation date")))
    {
        info->creation_date = temp_b_value->number;
    }

    return info;
}


bool parse_BEN_info_to_TR_info(Arena *arena, const BEN_pair *b_info, TR_info *info)
{
    BEN_value *temp_b_value;
    temp_b_value = get_BEN_value_by_key(b_info, "name");
    if(temp_b_value == NULL)
    {
        printf("ERROR: torrent file doesnt have essential fields");
        return false;
    }
    if (!sanitize_file_BEN_string(&temp_b_value))
    {
        return false;
    }
    info->name = BEN_string_to_C_string(arena, &temp_b_value->string);
    

    temp_b_value = get_BEN_value_by_key(b_info, "piece length");
    if(temp_b_value == NULL)
    {
        printf("ERROR: torrent file doesnt have essential fields");
        return false;
    }
    info->piece_length = temp_b_value->number;

    temp_b_value = get_BEN_value_by_key(b_info, "pieces");
    if(temp_b_value == NULL)
    {
        printf("ERROR: torrent file doesnt have essential fields");
        return false;
    }
    parse_BEN_pieces_to_TR_info(arena, &temp_b_value->string, info);

    temp_b_value = get_BEN_value_by_key(b_info, "files");
    if (temp_b_value != NULL)
    {
        parse_BEN_multifile_list_to_TR_info(arena, temp_b_value->list, info);
        if (info->files == NULL)
            return false;

        return true;
    } else if ((temp_b_value = get_BEN_value_by_key(b_info, "length")) != NULL)
    {
        info->files = arena_push_struct(arena, TR_file);
        info->files->length = temp_b_value->number;
        info->files->path = info->name;
        return true;
    } else 
    {
        printf("ERROR: torrent file doesnt have essential fields");
        return false;
    }
}

bool parse_BEN_announce_to_TR_info(Arena *arena, const BEN_pair *pair, TR_info *info)
{
    BEN_value *announce_list;
    BEN_list *tier_list, *track_list;
    int idx = 0, tier = 0, trackers_count = 0;

    info->trackers_count = 0;

    

    if ((announce_list = get_BEN_value_by_key(pair, "announce-list")))
    {
        tier_list = announce_list->list;
        while (tier_list)
        {
            track_list = tier_list->value->list;
            while(track_list)
            {
                trackers_count++;
                track_list = track_list->next;
            }
            tier_list = tier_list->next;
        }

        info->trackers = arena_push_array(arena, TR_tracker, trackers_count);
        info->trackers_count = trackers_count;

        tier_list = announce_list->list;
        while (tier_list)
        {
            track_list = tier_list->value->list;
            while(track_list)
            {
                info->trackers[idx].announce = BEN_string_to_C_string(arena, &track_list->value->string);
                info->trackers[idx].tier = tier;
                idx++;
                track_list = track_list->next;
            }
            tier++;
            tier_list = tier_list->next;
        }
        return true;
    } else if ((announce_list = get_BEN_value_by_key(pair, "announce")))
    {   
        info->trackers = arena_push_struct(arena, TR_tracker);
        info->trackers_count++;
        info->trackers[0].announce = BEN_string_to_C_string(arena, &announce_list->string);
        info->trackers[0].tier = 0;
        trackers_count++;
        info->trackers_count = trackers_count;
        return true;
    } else 
    {
        printf("ERROR: torrent file doesnt have essential fields");
        return false;
    }
}


void parse_BEN_pieces_to_TR_info(Arena *arena, BEN_string *b_str, TR_info *info)
{
    info->pieces = arena_push_array(arena, uint8_t, b_str->length);
    memcpy(info->pieces, b_str->data, b_str->length);

    info->pieces_string_length = b_str->length;
    return;
}

void parse_BEN_multifile_list_to_TR_info(Arena *arena, BEN_list *b_list, TR_info *info)
{
    int i = 0, file_count = 0;
    BEN_value *temp_val;
    BEN_list *file_list = b_list;

    while(file_list)
    {
        file_count++;
        file_list = file_list->next;
    }

    info->files = arena_push_array(arena, TR_file,  file_count);
    info->files_count = file_count;





    file_list = b_list;
    while (file_list)
    {
        temp_val = get_BEN_value_by_key(file_list->value->dict, "length");
        if (temp_val == NULL)
        {
            info->files = NULL;
            printf("ERROR: missing essential fields\n");
            return;
        }
        info->files[i].length = temp_val->number;

        temp_val = get_BEN_value_by_key(file_list->value->dict, "path");
        if (temp_val == NULL)
        {
            info->files = NULL;
            printf("ERROR: missing essential fields\n");
            return;
        }
        info->files[i].path = parse_BEN_list_to_path_C_string(arena, temp_val->list);
        if (info->files[i].path == NULL)
        {
            info->files = NULL;
            return;
        }
        i++;
        file_list = file_list->next;
    }
}

char *parse_BEN_list_to_path_C_string(Arena *arena, BEN_list *b_list)
{
    int path_length = 0, count = 0;
    char *path_str;
    char *cursor;

    BEN_list *path_list = b_list;
    while (path_list)
    {
        if (!sanitize_file_BEN_string(&path_list->value->string))
        {
            printf("ERROR: malicious filepath\");
            return NULL;
        }
        path_length += path_list->value->string.length;
        path_list = path_list->next;
        count++;
    }
    path_length += count + 1; //for "/" symbol and NULL terminator 
    
    path_str = arena_push_array(arena, char, path_length);
    path_str[0] = '\0';
    cursor = path_str;
    path_list = b_list;
    while (path_list)
    {
        *cursor++ = '/';
        memcpy(cursor, (char *) path_list->value->string.data, path_list->value->string.length);
        cursor += path_list->value->string.length;
        path_list = path_list->next;
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

bool sanitize_file_BEN_string(BEN_string *path)
{
    int i, start_pos = 0, end_pos = path->length;
    if ( path->length == 0 ||  (path->data[0] == '.' && path->length ==1))
        return false;

    for (i = 0; i < path->length; i++)
        if (reserved_table[(unsigned char)path->data[i]])
        path->data[i] = '_';

    while (start_pos < path->length && path->data[start_pos] == ' ')
    start_pos++;

    while (end_pos > start_pos && (path->data[end_pos - 1] == ' ' || path->data[end_pos - 1] == '.'))
    end_pos--;

    if (end_pos == start_pos)
       return false;
   path->length = end_pos;
   path->data += start_pos; 
   path->length -= start_pos;
   return true;

}
