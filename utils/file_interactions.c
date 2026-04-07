#include "file_interactions.h"
#include <stdio.h>
#include <stdlib.h>

file_content_buffer read_bencoded_file(char *file_path)
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
