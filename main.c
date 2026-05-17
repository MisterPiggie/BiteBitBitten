#include <endian.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include "repl/repl.h"
#include "announcer/announcer.h"
#include "files/file_interactions.h"
#include "parser/parser.h"
#include "init/init_session.h"
#include "types/types.h"
#include "utils/str_utils.h"

int main(void) 
{
    char  line[MAX_CHARS];
    char *argv[MAX_ARGS];
    CL_session *session = malloc(sizeof(CL_session));

    session = init_CL_session();

    while (1) {
        printf("bbb=> ");
        fflush(stdout);

        if (!fgets(line, sizeof(line), stdin)) break; 

        /* trim trailing newline */
        line[strcspn(line, "\n")] = '\0';
        if (line[0] == '\0') continue;                 
        if (strcmp(line, "exit") == 0) break;

        int argc = tokenize(line, argv, MAX_ARGS);
        dispatch(session, argc, argv);
    }

    puts("Bye!");
    return 0;
}
    
    //
    // for (i = 0; i < info->trackers_length; i++)
    // {
    //     err = UDP_send_connect_req(session->udp_socket, &torrent->tracks[i]);
    //     if (err != 0)
    //     {
    //         printf("ERROR: send connect req\n");
    //         continue;
    //     }
    //     printf("Connection ID before recv: %lu\n", torrent->tracks[i].connection_id);
    //
    //     err = UDP_recv_connect_req(session->udp_socket, &torrent->tracks[i]);
    //     if (err != 0)
    //     {
    //         printf("ERROR: send recv req\n");
    //         continue;
    //     }
    //
    //     printf("Connection ID after recv: %lu\n", torrent->tracks[i].connection_id);
    //     torrent->active_tracker_idx = i;
    //     UDP_request req;
    //     construct_UDP_request(&req, torrent);
    //     if(UDP_send_announce_req(session, torrent, &req) != 0)
    //         printf("ERROR: send announce req\n");
    //
    //     if(UDP_recv_announce_resp(session->udp_socket) != 0)
    //         printf("ERROR: recv announce resp\n");
    // }
    //
