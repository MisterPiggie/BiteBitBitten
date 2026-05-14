#include "file_interactions.h"
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

file_content_buffer *read_BEN_file(char *file_path)
{
    file_content_buffer buffer = {0};
    FILE *fp = fopen(file_path, "rb");
    if (!fp)
    {
        return buffer;
    }

    fseek(fp, 0, SEEK_END);
    buffer.size = ftell(fp);
    rewind(fp);

    buffer.data = malloc(buffer.size);
    fread(buffer.data, sizeof(unsigned char), buffer.size, fp);

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
