#include <stdio.h>
#include <stdbool.h>
#include "../types/types.h"

bool get_config_data(CL_session *session);
char *make_config_dir(void);
file_content_buffer read_BEN_file(char *file_path);
bool save_state_in_file(BEN_value *value);


