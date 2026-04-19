#include "file_interactions.h"
#include <stdio.h>
#include <stdlib.h>

file_content_buffer read_BEN_file(char *file_path)
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
