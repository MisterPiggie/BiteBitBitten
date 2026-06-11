#include "init_session.h"
#include "../announcer/announcer.h"
#include "../parser/parser.h"
#include "../utils/str_utils.h"
#include "../files/file_interactions.h"
#include <bits/types/struct_timeval.h>
#include <stdio.h>
#include <string.h>
#include <stddef.h>
#include <stdlib.h>
#include <pwd.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <dirent.h>

#define MAX_TORRENTS 128

void init_CL_session(CL_session *session, Arena *arena)
{
    const char *config_path = get_config_dir_path(arena);
    const char *home_path = get_home_dir_path();

    session->config_dir_path = arena_push_strf(arena, "%s/%s",config_path, "bbb");
    session->config_file_path = arena_push_strf(arena, "%s/%s",session->config_dir_path, "config.pig");
    session->resume_dir_path = arena_push_strf(arena, "%s/%s",session->config_dir_path, "resume");
    session->torrent_dir_path = arena_push_strf(arena, "%s/%s",session->config_dir_path, "torrent");
    session->download_folder_path = arena_push_strf(arena, "%s/%s",home_path, "bbb_download");

    session->torrents = arena_push_array(arena, TR_torrent* , MAX_TORRENTS);
    session->torrents_count = 0;

    generate_peer_id(session->peer_id);

    session->main_arena = arena;
    
    init_reserved_table();
    return;
}

const char *get_config_dir_path(Arena *arena)
{
    static const char *config_path = NULL;

    if (config_path)
        return config_path;

    config_path = getenv("XDG_CONFIG_HOME");
    if (config_path)
        return config_path;

    config_path = get_home_dir_path();

    return arena_push_strf(arena, "%s/%s", config_path, ".config");

}
 

const char *get_home_dir_path(void)
{
    static const char *home_path = NULL;

    if (home_path) 
        return home_path;

    home_path = getenv("HOME");
    if (home_path)
        return home_path;

    struct passwd *pw = getpwuid(getuid());
    if (pw)
    {
        home_path = strdup(pw->pw_dir);
        endpwent();
        return home_path;
    }
    endpwent();

    home_path = "";

    return home_path;
}


TR_torrent *init_TR_torrent(BEN_pair *pair)
{
    int i;

    Arena torrent_arena = arena_create(MB(100));
    TR_torrent *torrent = arena_push_struct(&torrent_arena, TR_torrent);

    torrent->info = BEN_pairs_to_TR_info(&torrent_arena, pair);
    if (!torrent->info)
    {
        arena_destroy(&torrent_arena);
        return NULL;
    }
    fill_in_calculated_field_in_TR_info(torrent->info);

    torrent->downloaded = 0;
    torrent->uploaded = 0;

    torrent->swarm = arena_push_struct(&torrent_arena, TR_swarm);
    init_TR_swarm(torrent->swarm, &torrent_arena, torrent->info->pieces_count, torrent->info->piece_length);

    torrent->tracker_count = torrent->info->trackers_count;

    torrent->tracks = arena_push_array(&torrent_arena, NET_tracker, torrent->tracker_count);

    for (i = 0; i < torrent->tracker_count; i++)
    {
        if(tracker_string_to_NET_tracker(&torrent_arena, torrent->info->trackers[i].announce, &torrent->tracks[i]) != 0)
        {
            printf("ERROR: malformed url\n");
            arena_destroy(&torrent_arena);
            return NULL;
        }
    }
    torrent->arena = torrent_arena;
    torrent->state = TORRENT_DOWNLOADING;

    return torrent;
    
}

void init_TR_swarm(TR_swarm *swarm, Arena *arena, int pieces_count, uint32_t piece_length)
{
    int bytes_count = (pieces_count + 7) / 8;
    swarm->bitfield_slab = arena_push_array_zero(arena, uint8_t, 50 * bytes_count);
    swarm->bytes_count = bytes_count;
    swarm->piece_buf_slab = arena_push_array_zero(arena, uint8_t, 50 * piece_length);

    for (int i = 0; i < 200; i++)
        swarm->peer_pool[i] = arena_push_struct(arena, TR_peer);
    for (int i = 0; i < 50; i++)
        swarm->peers[i] = arena_push_struct(arena, TR_peer);

    swarm->pool_count  = 0;
    swarm->peers_count = 0;
}

void init_saved_torrents(CL_session *session)
{
    struct dirent *entry;
    DIR *dir = opendir(session->torrent_dir_path);
    if (dir == NULL)
    {
        printf("ERROR: couldnt init saved torrents\n");
        return;
    }

    char buf[1024];

    while((entry = readdir(dir)) != NULL)
    {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
            continue;
        snprintf(buf, sizeof(buf), "%s/%s", session->torrent_dir_path, entry->d_name);
        printf("%s\n", buf);
        CMD_add_init(session, buf); 
    }

    return;
}

void CMD_add_init(CL_session *session, char *path)
{
    file_content_buffer *buffer;
    BEN_parser          *parser;
    BEN_value           *dict;
    TR_torrent          *tmp_torrent;

    Arena scratch_arena = arena_create(MB(100));

    buffer = read_BEN_file(&scratch_arena, path);
    if (!buffer)
    {
        arena_destroy(&scratch_arena);
        return;
    }
    
    parser = init_BEN_parser(&scratch_arena, buffer);

    dict = parse_dict(&scratch_arena, parser);
    if (!dict)
    {
        arena_destroy(&scratch_arena);
        return;
    }

    tmp_torrent = init_TR_torrent(dict->dict);
    if (!tmp_torrent)
    {
        arena_destroy(&scratch_arena);
        return;
    }

    get_info_hash(parser, tmp_torrent->info->info_hash);

    session->torrents[session->torrents_count++] = tmp_torrent;

    session->torrents[session->torrents_count-1]->download_path =
        arena_push_strf(&tmp_torrent->arena, "%s/%s", session->download_folder_path, tmp_torrent->info->name);

    replace_spaces_with(session->torrents[session->torrents_count-1]->download_path, '_');

    for (int i; i < tmp_torrent->info->files_count; i++)
    {
        if(!allocate_file(tmp_torrent->download_path,tmp_torrent->info->files[i].path, tmp_torrent->info->files[i].length))
        {
            arena_destroy(&tmp_torrent->arena);
            arena_destroy(&scratch_arena);
            return;
        }
    }
    arena_destroy(&scratch_arena);
    tmp_torrent->state = TORRENT_NEEDS_CHECK;
    tmp_torrent->torrent_file_path = arena_push_strf(&tmp_torrent->arena, path);
    

}
