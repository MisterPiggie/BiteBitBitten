#include "init_session.h"
#include "../announcer/announcer.h"
#include "../parser/parser.h"
#include <bits/types/struct_timeval.h>
#include <stdio.h>
#include <string.h>
#include <stddef.h>
#include <stdlib.h>
#include <pwd.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define MAX_TORRENTS 128

void init_CL_session(CL_session *session, Arena *arena)
{
    const char *config_path = get_config_dir_path(arena);
    const char *home_path = get_home_dir_path();

    session->config_dir_path = arena_push_strf(arena, "%s/%s",config_path, "bbb");
    session->config_file_path = arena_push_strf(arena, "%s/%s",config_path, "bbb");
    session->resume_dir_path = arena_push_strf(arena, "%s/%s",config_path, "bbb");
    session->torrent_dir_path = arena_push_strf(arena, "%s/%s",config_path, "bbb");
    session->download_folder_path = arena_push_strf(arena, "%s/%s",home_path, "bbb_download");

    session->torrents = arena_push_array(arena, TR_torrent* , MAX_TORRENTS);
    session->torrents_count = 0;

    generate_peer_id(session->peer_id);

    session->main_arena = arena;
    
    session->udp_socket = socket(AF_INET, SOCK_DGRAM, 0);
    if (session->udp_socket == -1)
        return;
    struct timeval tv = { .tv_sec = 3, .tv_usec = 0};
    setsockopt(session->udp_socket, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
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

    Arena torrent_arena = arena_create(MB(10));
    TR_torrent *torrent = arena_push_struct(&torrent_arena, TR_torrent);
    torrent->arena = torrent_arena;

    torrent->info = arena_push_struct(&torrent_arena, TR_info);
    torrent->info = BEN_pairs_to_TR_info(&torrent_arena, pair);
    if (!torrent->info)
    {
        arena_destroy(&torrent_arena);
        return NULL;
    }
    fill_in_calculated_field_in_TR_info(torrent->info);

    torrent->downloaded = 0;
    torrent->uploaded = 0;
    torrent->tcp_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (torrent->tcp_socket == -1)
    {
        arena_destroy(&torrent_arena);
        return NULL;
    }

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
        torrent->tracks[i].key = get_random_u32();
    }

    return torrent;
    
}

