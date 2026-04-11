#include <stdio.h>
#include <stdbool.h>
#include "../types/types.h"

file_content_buffer read_bencoded_file(char *file_path);
bool save_state_in_file(bencode_value *value);


