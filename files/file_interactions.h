#include <stdio.h>
#include <stdbool.h>
#include <sys/types.h>
#include "../types/types.h"

file_content_buffer read_BEN_file(char *file_path);
bool save_state_in_file(BEN_value *value);

bool make_dir(char *path, int permissions);
bool make_dir_recursive(char *path, int permissions);
