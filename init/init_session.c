#include "init_session.h"
#include <bits/types/struct_timeval.h>
#include <stdio.h>
#include <string.h>
#include <stddef.h>
#include <stdlib.h>
#include <pwd.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>


void init_CL_session(CL_session *session)
{
    const char* config_path = get_config_dir_path();

    session->config_dir_path = build_path(config_path, "bbb");
    session->config_file_path = build_path(session->config_dir_path, "config.pig");
    session->resume_dir_path = build_path(session->config_dir_path, "resume");
    session->torrent_dir_path = build_path(session->config_dir_path, "torrent");
    
    return;
}

int init_CL_announcer(CL_announcer *ann)
{
    ann->udp_socket = socket(AF_INET, SOCK_DGRAM, 0);
    if (ann->udp_socket == -1)
        return -1;
    struct timeval tv = { .tv_sec = 3, .tv_usec = 0};
    setsockopt(ann->udp_socket, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
    return 0;
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


