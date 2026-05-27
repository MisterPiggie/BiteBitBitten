#include <endian.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include "repl/repl.h"
#include "announcer/announcer.h"
#include "files/file_interactions.h"
#include "parser/parser.h"
#include "init/init_session.h"
#include "types/types.h"
#include "utils/str_utils.h"
#include "arena/arena.h"

int main(void) {
    char  line[MAX_CHARS];
    char *argv[MAX_ARGS];
    Arena main_arena = arena_create(GB(1));
    CL_session session = {0};
    EV_loop loop;
    
    init_CL_session(&session, &main_arena);
    init_saved_torrents(&session);
    init_EV_loop(&loop, &session);


    while (1) {
        printf("bbb=> ");
        fflush(stdout);

        if (!fgets(line, sizeof(line), stdin)) break; 

        line[strcspn(line, "\n")] = '\0';
        if (line[0] == '\0') continue;                 
        if (strcmp(line, "exit") == 0) 
        {
            for (int i = 0; i < session.torrents_count; i++)
            {
                arena_destroy(&session.torrents[i]->arena);
            }
            arena_destroy(&main_arena);
            break;
        }

        int argc = tokenize(line, argv, MAX_ARGS);
        dispatch(&session, argc, argv);
    }

    puts("Bye!");
    return 0;
}
    
    
