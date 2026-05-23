#include "str_utils.h"
#include <ctype.h>
#include <stddef.h>
#include <string.h>

void trim_spaces(char *str)
{
    if (str == NULL || *str == '\0')
        return;

    char *start = str;
    char *end;

    while (isspace((unsigned char)*start))
        start++;
    
    if (*start == '\0')
    {
        *str = '\0';
        return;
    }

    end = start + strlen(start) - 1;

    while (end > start && isspace((unsigned char)*end)) end--;

    *(end + 1) = '\0';

    if (start != str)
    {
        memmove(str, start, end - start + 2);
    }
}

void replace_spaces_with(char *str, char replacement)
{
    if (str == NULL || *str == '\0')
        return;

    char *start = str;
    while (*start != '\0')
    {
        if (*start == ' ')
            *start = replacement;
        start++;
    }
}

void replace_bad_chars_with(char *str, char replacement)
{
    if (str == NULL || *str == '\0')
        return;

    char *start = str;
    while (*start != '\0')
    {
        if (is_bad_char(*start))
            *start = replacement;
        start++;
    }

}

int is_bad_char(char ch) {
    static const char reserved[] = "\\/:*?\"<>|";
    unsigned char c = (unsigned char)ch;

    if (c < 0x20) return 1;   
    if (c == 0x7F) return 1; 

    for (int i = 0; reserved[i]; i++)
        if (c == (unsigned char)reserved[i]) return 1;

    return 0;
}

void truncate_filename(char *str)
{
    if (strlen(str) > 255)
        str[255] = '\0';
}

int contains_suffix(char *str, char *suffix)
{
    int i;
    int str_len = strlen(str),
        suffix_len = strlen(suffix);
    if (str_len < suffix_len)
        return 0;

    for (i = 0; i < suffix_len; i++)
    {
        if (suffix[suffix_len - 1 - i] !=  str[str_len - 1 - i]
            return 0;
    }

    return 1;
}