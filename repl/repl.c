#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include "../types/types.h"
#include "../files/file_interactions.h"
#include "../parser/parser.h"
#include "../init/init_session.h"
#include "../announcer/announcer.h"
#include "../utils/str_utils.h"
#include "../init/init_session.h"
#include "repl.h"


int tokenize(char *line, char **argv, int max_args)
{
    int argc = 0;
    char *token = strtok(line, " \t\n");

    while (token && argc < max_args)
    {
        argv[argc++] = token;
        token = strtok(NULL, " \t\n");
    }

    return argc;
}

void CMD_add(CL_session *session, int argc, char **argv)
{
    file_content_buffer *buffer;
    BEN_parser          *parser;
    BEN_value           *dict;
    TR_torrent          *tmp_torrent;



    if (argc < 2)
    {
        puts("USAGE: add <.torrent filepath>\n");
        return;
    }

    Arena scratch_arena = arena_create(MB(100));

    buffer = read_BEN_file(&scratch_arena, argv[1]);
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
    if (is_duplicate_torrent(tmp_torrent->info->info_hash, session))
    {
        printf("Torrent is already added\n");
        arena_destroy(&tmp_torrent->arena);
        arena_destroy(&scratch_arena);
        return;
    }

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
    tmp_torrent->state = TORRENT_DOWNLOADING;
    arena_destroy(&scratch_arena);
    tmp_torrent->torrent_file_path = arena_push_strf(&tmp_torrent->arena,"%s/%s", session->torrent_dir_path, argv[1]);
    
    if (!copy_torrent_file(argv[1], tmp_torrent->torrent_file_path))
    {
        printf("ERROR: couldnt save torrent file after parsing\n");
        return;
    }


}

void CMD_print(CL_session *session, int argc, char **argv)
{
    int i;
    int id;
    TR_torrent *torrent;

    if (argc != 2)
    {
        printf("Usage: print <id>\n");
        return;
    }

    if (id_from_arg(argv[1], &id) != 0)
    {
        printf("ERROR: '%s' is not a valid id\n", argv[1]);
        return;
    }

    id -= 1;

    if (id >= session->torrents_count)
    {
        printf("ERROR: no torrent with id %d\n", id + 1);
        return;
    }    

    torrent = session->torrents[id];

    printf("%d, %s\n", argc, argv[0]);
    printf("Torrent number %d\n", id + 1);
    printf("\tName: %s\n", torrent->info->name);
    printf("\tCreated by: %s\n", torrent->info->created_by);
    printf("\tComment: %s\n", torrent->info->comment);
    printf("\tCreation date: %ld\n", torrent->info->creation_date);
    printf("\tPieces length: %ld\n", torrent->info->piece_length);
    printf("\tTotal size: %ld\n", torrent->info->total_size);
    printf("\tAmount of files: %d\n", torrent->info->files_count);
    printf("\tFiles:\n");
    for (i = 0; i < torrent->info->files_count; i++)
    {
        printf("\t\tPath: %s\n", torrent->info->files[i].path);
        printf("\t\tLength: %ld\n", torrent->info->files[i].length);
        printf("\t\tFirst piece: %d\n", torrent->info->files[i].first_piece);
        printf("\t\tLast piece: %d\n", torrent->info->files[i].last_piece);
        printf("\t\tOffset: %ld\n", torrent->info->files[i].offset);
    }

    printf("\tTrackers:\n");
    for (i = 0; i < torrent->info->trackers_count; i++)
        printf("\t\t%s\n", torrent->info->trackers[i].announce);
    printf("\tHash info:");
    for(int j = 0; j < 20; j++) {
        printf("%02x", torrent->info->info_hash[j]);
    };
    printf("\n");
    printf("\tDownload path: %s\n", torrent->download_path);

}

void CMD_delete(CL_session *session, int argc, char **argv)
{
    int id;

    if (argc != 2)
    {
        printf("Usage: delete <id>\n");
        return;
    }

    if (id_from_arg(argv[1], &id) != 0)
    {
        printf("ERROR: '%s' is not a valid id\n", argv[1]);
        return;
    }

    id -= 1;

    if (id >= session->torrents_count)
    {
        printf("ERROR: no torrent with id %d\n", id + 1);
        return;
    }    

    remove(session->torrents[id]->torrent_file_path);

    arena_destroy(&session->torrents[id]->arena);
    session->torrents[id] = session->torrents[session->torrents_count - 1];
    session->torrents_count--;
}

void CMD_list(CL_session *session, int argc, char **argv)
{
    (void) argv;
    int i;
    if (argc > 1)
    {
        printf("USAGE: list\n");
        return;

    }
    if (session->torrents_count == 0)
    {
        printf("No torrents added.\n");
        return;
    }
    for (i = 0; i < session->torrents_count; i++)
    {
        printf("Torrent ID: %d\n", i+1);
        printf("\tName: %s\n", session->torrents[i]->info->name);
    }
    return;
}
void CMD_help(CL_session *session, int argc, char **argv)
{
    (void)session;
    (void)argc;
    (void)argv;
    printf("ByteBitBitten Torrent client commands:\n\tadd <filepath to .torrent file> - adding torrent to download\n\tdelete <id> - delete torrent with entered id\n\tlist - list all added torrents with id\n\tprint <id> - print extended information about torrent with entered id\n\thelp - see this message again");
    return;
}

void dispatch(CL_session *seesion, int argc, char *argv[]) 
{
    if (argc == 0) return;
    for (int i = 0; i < NUM_COMMANDS; i++) {
        if (strcmp(argv[0], CMD_commands[i].name) == 0) {
            CMD_commands[i].fn(seesion, argc, argv);
            return;
        }
    }
    printf("Unknown command: '%s'. Type 'help' for list.\n", argv[0]);
}

int id_from_arg(const char *str, int *out)
{
    char *end;
    errno = 0;
    long val = strtol(str, &end, 10);

    if (errno != 0 || end == str || *end != '\0')
        return -1;   
    if (val < 1)
        return -1;  

    *out = (int)val;
    return 0;
}

bool is_duplicate_torrent(uint8_t hash[20], CL_session *session)
{
    int i;
    for (i = 0; i < session->torrents_count; i++)
    {
        if (memcmp(hash, session->torrents[i]->info->info_hash, 20) == 0)
            return true;
    }
    return false;
}
