#include "init_session.h"
#include "../announcer/announcer.h"
#include <bits/types/struct_timeval.h>
#include <stdio.h>
#include <string.h>
#include <stddef.h>
#include <stdlib.h>
#include <pwd.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>


CL_session *init_CL_session(void)
{
    const char* config_path = get_config_dir_path();
    CL_session *session = malloc(sizeof(CL_session));

    session->config_dir_path = build_path(config_path, "bbb");
    session->config_file_path = build_path(session->config_dir_path, "config.pig");
    session->resume_dir_path = build_path(session->config_dir_path, "resume");
    session->torrent_dir_path = build_path(session->config_dir_path, "torrent");

    generate_peer_id(session->peer_id);
    
    session->udp_socket = socket(AF_INET, SOCK_DGRAM, 0);
    if (session->udp_socket == -1)
        return session;
    struct timeval tv = { .tv_sec = 3, .tv_usec = 0};
    setsockopt(session->udp_socket, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
    return session;
}

char const *get_config_dir_path(void)
{
    static const char *config_path = NULL;

    if (config_path)
        return config_path;

    config_path = getenv("XDG_CONFIG_HOME");
    if (config_path)
        return config_path;

    config_path = get_home_dir_path();

    config_path = build_path(config_path, ".config");

    return config_path;

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


char *build_path(const char *base_path, const char *sub_path)
{
    size_t len = strlen(base_path) + strlen(sub_path) + 2;
    char *out_path = malloc(len * sizeof(char));
    snprintf(out_path, len, "%s/%s", base_path, sub_path);
    return out_path;
}

TR_torrent *init_TR_torrent(TR_info *info)
{
    int i;

    TR_torrent *torrent = malloc(sizeof(TR_torrent));
    torrent->downloaded = 0;
    torrent->uploaded = 0;

    torrent->info = info;
    torrent->tracker_count = info->trackers_length;

    torrent->tracks = malloc(sizeof(NET_tracker) * torrent->tracker_count);
    if (torrent->tracks == NULL)
        return;

    for (i = 0; i < torrent->tracker_count; i++)
    {
        if(tracker_string_to_NET_tracker(info->trackers[i].announce, &torrent->tracks[i]) != 0)
            return;
        torrent->tracks[i].key = get_random_u32();
    }
    
}

