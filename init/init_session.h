#include "../types/types.h"
#include "../arena/arena.h"

//Init funcs for Client
const char *get_config_dir_path(Arena *arena);
void init_CL_session(CL_session *session, Arena *arena);

//Helper init funcs 
const char *get_home_dir_path(void);

TR_torrent *init_TR_torrent(BEN_pair *pair);
