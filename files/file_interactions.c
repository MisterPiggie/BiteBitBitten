#include "file_interactions.h"
#include <errno.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include "../arena/arena.h"

file_content_buffer *read_BEN_file(Arena *arena, char *file_path)
{
    FILE *fp;
    size_t bytes_read;
    file_content_buffer *buffer = arena_push_struct(arena, file_content_buffer);

    buffer->data = NULL;
    buffer->size = 0;

    fp = fopen(file_path, "rb");
    if (!fp)
    {
        puts("ERROR: could not open file");
        return NULL;
    }

    fseek(fp, 0, SEEK_END);
    buffer->size = ftell(fp);
    rewind(fp);

    buffer->data = arena_push_array(arena, unsigned char, buffer->size);

    bytes_read = fread(buffer->data, sizeof(unsigned char), buffer->size, fp);
    if (bytes_read != buffer->size)
    {
        puts("ERROR: could not read .torrent file entaerly");
        return NULL;
    }

    fclose(fp);
    return buffer;
}

bool copy_torrent_file(char *source, char *dest)
{
    char ch;
    FILE *source_fp,  *dest_fp;
    if ((source_fp = fopen(source, "rb")) == NULL)
    {
        return false;
    }
    
    if ((dest_fp = fopen(dest, "wb")) == NULL)
    {
        fclose(source_fp);
        return false;
    }

    while((ch = getc(source_fp)) != EOF)
        putc(ch, dest_fp);
    
    fclose(source_fp);
    fclose(dest_fp);

    return true;

}

bool make_dir(char *path, int permissions)
{
    return mkdir(path, permissions) == 0 || errno == EEXIST;
}

bool make_dir_recursive(char *path, int permissions)
{
    bool ok;
    char *path_copy = strdup(path);
    char *tmp = path_copy;

    while (*tmp != '\0')
    {
        if (*tmp == '/')
        {
            *tmp = '\0'; //change '/' to null to use this for make_dir 

            if (*path_copy != '\0')
                make_dir(path_copy, permissions);

            *tmp = '/'; // change null back to '/'
        }

        tmp++;
    }
    ok = make_dir(path_copy, permissions);

    free(path_copy);
    return ok;
}
