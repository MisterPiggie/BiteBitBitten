#include "parser.h"
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>


void init_bencode_parser(file_content_buffer buffer, bencode_parser *parser)
{
    parser->buffer = buffer;
    parser->cursor = 0;
}

bencode_value *parse_file_content_buffer(bencode_parser *parser)
{
    bencode_value *ret_dict;

    ret_dict = parse_dict(parser);

    return ret_dict;
}


bencode_value *parse_dict(bencode_parser *parser)
{
    bencode_string key;
    bencode_value *value; 

    bencode_value *return_dict= malloc(sizeof(bencode_value));

    return_dict->type = BENCODE_DICT;
    return_dict->dict.bencode_pairs = NULL;
    return_dict->dict.count = 0;

    consume(parser);

    while(peek(parser) != 'e')
    {
        key = parse_raw_string(parser);
        value = parse_value(parser);
        printf("%d\n", return_dict->dict.count); //debug
        printf("%.*s\n", (int) key.length, key.data); //debug
        printf("%p\n", value); //debug

        return_dict->dict.bencode_pairs = realloc(
                return_dict->dict.bencode_pairs,
                sizeof(bencode_pair) * (return_dict->dict.count + 1)
               );
        return_dict->dict.bencode_pairs[return_dict->dict.count].key = key;
        return_dict->dict.bencode_pairs[return_dict->dict.count].value = value;
        return_dict->dict.count++;
    }
    consume(parser); // consume final e
    return return_dict;
}

bencode_value *parse_string(bencode_parser *parser)
{
    bencode_value *return_val = malloc(sizeof(bencode_value));
    return_val->type = BENCODE_STRING;
    return_val->string = parse_raw_string(parser);

    return return_val;
}

bencode_value *parse_num(bencode_parser *parser)
{
    int64_t num; 
    char *end;
    bencode_value *return_val = malloc(sizeof(bencode_value));

    consume(parser); //consume i
    num = strtol((char *) parser->buffer.data + parser->cursor, &end, 10);
    
    return_val->type = BENCODE_NUMBER;
    return_val->number = num;

    parser->cursor = end - (char *) parser->buffer.data; //might be problem with cast

    consume(parser); //consume e
    
    return return_val;
}

bencode_value *parse_list(bencode_parser *parser)
{
    bencode_value *return_val = malloc(sizeof(bencode_value));
    return_val->type = BENCODE_LIST;
    return_val->list.items = NULL;
    return_val->list.count = 0;

    consume(parser); //consume l

    while(peek(parser) != 'e')
    {
        return_val->list.items = realloc(
                return_val->list.items, 
                sizeof(bencode_value *) * (return_val->list.count + 1)
        );

        return_val->list.items[return_val->list.count++] = parse_value(parser);
    }

    consume(parser); //consume e

    return return_val;
}

unsigned char peek(bencode_parser *parser)
{
    return parser->buffer.data[parser->cursor];
}

unsigned char consume(bencode_parser *parser)
{
     return parser->buffer.data[parser->cursor++]; // might be problem
}


bencode_string parse_raw_string(bencode_parser *parser)
{
    char *delimeter;
    bencode_string raw_str;

    raw_str.length = strtol((char *) parser->buffer.data + parser->cursor, &delimeter, 10); //might be problem with cast
    parser->cursor = delimeter - (char *) parser->buffer.data; //might be problem with cast
    consume(parser);

    raw_str.data = parser->buffer.data + parser->cursor;
    parser->cursor += raw_str.length;
    return raw_str;
}

bencode_value *parse_value(bencode_parser *parser)
{
    switch (peek(parser))
    {
        case 'i': return parse_num(parser);
        case 'l': return parse_list(parser);
        case 'd': return parse_dict(parser);
        default: return parse_string(parser);
    }
}


void get_info_value_offset(bencode_parser *parser, int *begining, int *end)
{
    bencode_string key; 

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

void skip_value(bencode_parser *parser)
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


