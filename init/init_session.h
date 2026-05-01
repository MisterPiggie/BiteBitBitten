#include "../types/types.h"

//Init funcs for Client
static char const *get_config_dir_path(void);
void init_CL_session(CL_session *session);

//Helper init funcs 
char *build_path(const char *base_path, const char *sub_path);
const char *get_home_dir_path(void);
const char *get_home_dir_path_from_pwd(void);
