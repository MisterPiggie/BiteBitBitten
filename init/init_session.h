#include "../types/types.h"

//Init funcs for Client
const char *get_config_dir_path(void);
CL_session *init_CL_session(void);

//Helper init funcs 
char *build_path(const char *base_path, const char *sub_path);
const char *get_home_dir_path(void);
const char *get_home_dir_path_from_pwd(void);

TR_torrent *init_TR_torrent(TR_info *info);
void free_TR_torrent(TR_torrent *torrent);
