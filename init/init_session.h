#include "../types/types.h"
#include "../arena/arena.h"

//Init funcs for Client
const char *get_config_dir_path(Arena *arena);
void init_CL_session(CL_session *session, Arena *arena);
void init_saved_torrents(CL_session *session);
void CMD_add_init(CL_session *session, char *path);

//Helper init funcs 
const char *get_home_dir_path(void);

TR_torrent *init_TR_torrent(BEN_pair *pair);
void init_TR_swarm(TR_swarm *swarm, Arena *arena, int pieces_count, uint32_t piece_length);
