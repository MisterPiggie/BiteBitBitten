#include "file_interactions.h"

int read_bencoded_file(char *file_path)
{
    FILE *fp = fopen(file_path, "rb");
    if (!fp)
    {
        return -1;
    }
    fclose(fp);
    return 0;
}
