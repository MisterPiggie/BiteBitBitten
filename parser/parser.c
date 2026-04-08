#include "parser.h"

bencode_pairs parse_file_content_buffer(file_content_buffer buffer)
{
    bencode_pairs pairs_list = {0};    
    int i = 0;
    while(i < buffer.size)
    {
        switch (buffer.data[i])
        {
            case 'd': pairs_list.bencode_pairs[pairs_list.count++] = parse_dict(buffer);
                      break;
            case 'i': pairs_list.bencode_pairs[pairs_list.count++] = parse_num(buffer);
                      break;
            case 'l': pairs_list.bencode_pairs[pairs_list.count++] = parse_list(buffer);
                      break;
            case '1': case '2': case '3': case '4': case '5':
            case '6': case '7': case '8': case '9':
                      pairs_list.bencode_pairs[pairs_list.count++] = parse_string(buffer)
        }
    }
}
