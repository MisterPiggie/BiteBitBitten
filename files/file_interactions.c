#define _GNU_SOURCE
#include "file_interactions.h"
#include <errno.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include "../arena/arena.h"
#include "../utils/str_utils.h"
#include "../types/types.h"

file_content_buffer *read_BEN_file(Arena *arena, char *file_path)
{
    if (!contains_suffix(file_path, ".torrent"))
    {
        printf("ERROR: file provided is not torrent\n");
        return NULL;
    }

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
    FILE *source_fp, *dest_fp;
    char buffer[65536]; 
    size_t bytes_read;
    bool ok;

    if ((source_fp = fopen(source, "rb")) == NULL)
    {
        return false;
    }

    if ((dest_fp = fopen(dest, "wb")) == NULL)
    {
        fclose(source_fp);
        return false;
    }

    while ((bytes_read = fread(buffer, 1, sizeof(buffer), source_fp)) > 0)
    {
        if (fwrite(buffer, 1, bytes_read, dest_fp) != bytes_read)
        {
            fclose(source_fp);
            fclose(dest_fp);
            return false;
        }
    }

    ok = !ferror(source_fp);

    fclose(source_fp);
    fclose(dest_fp);

    if (!ok)
        remove(dest);

    return ok;
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

bool allocate_file(char *download_path, char *file_path, uint64_t length)
{
    char buf[4096];
    snprintf(buf, sizeof(buf), "%s%s", download_path, file_path);
    char *last_slash = strrchr(buf, '/');

    if (last_slash)
    {
        *last_slash = '\0';
        if (!make_dir_recursive(buf, 0755))
        {
            printf("ERROR: couldnt make folders for torrent\n");
            return false;
        }
        *last_slash = '/';
    }


    int fd = open(buf, O_CREAT | O_WRONLY, 0644);
    if (fd == -1)
    {
        printf("ERROR: couldnt make file\n");
        return false;
    }
    
    if (fallocate(fd, 0, 0, length) != 0)
    {
        printf("ERROR: couldnt allocate space for file\n");
        close(fd);
        return false;
    }
    close(fd);
    return true;
}
