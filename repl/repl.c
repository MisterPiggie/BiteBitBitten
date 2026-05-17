#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "../types/types.h"
#include "../files/file_interactions.h"
#include "../parser/parser.h"
#include "../init/init_session.h"
#include "../announcer/announcer.h"
#include "../utils/str_utils.h"
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
    TR_info             *info;
    TR_torrent          **tmp_torrent;



    if (argc < 2)
    {
        puts("USAGE: add <.torrent filepath>\n");
        return;
    }

    buffer = read_BEN_file(argv[1]);
    if (!buffer)
        return;
    
    parser = init_BEN_parser(buffer);
    if (!parser)
    {
        free(buffer->data);
        free(buffer);
        return;
    }

    dict = parse_dict(parser);
    if (!dict)
    {
        printf("dict failed\n");
        free(parser);
        free(buffer->data);
        free(buffer);
        return;
    }

    info = BEN_pairs_to_TR_info(&dict->dict);
    if (!info)
    {
        free(buffer->data);
        free(buffer);
        free(parser);
        free_BEN_value(dict);
        return;
    }


    fill_in_calculated_field_in_TR_info(info);
    get_info_hash(parser, info->info_hash);

    free(buffer->data);
    free(buffer);
    free_BEN_value(dict);
    free(parser);
    
    tmp_torrent = realloc(session->torrents, ++session->torrents_count * sizeof(TR_torrent));
    if (!tmp_torrent)
    {
        printf("ERROR: not enough memory to add another torrent\n");
        --session->torrents_count;
        return;
    }
    session->torrents = tmp_torrent;
    session->torrents[session->torrents_count-1] = init_TR_torrent(info);
    if (session->torrents[session->torrents_count-1] == NULL)
    {
        printf("ERROR: not enough memory to add another torrent\n");
        --session->torrents_count;
        return;
    }
    printf("Torrent added successfully\n");

    session->torrents[session->torrents_count-1]->download_path =
        build_path(session->download_folder_path, session->torrents[session->torrents_count-1]->info->name);
    replace_spaces_with(session->torrents[session->torrents_count-1]->download_path, '_');

    return;
}

void CMD_print(CL_session *session, int argc, char **argv)
{
    int i, j;
    for (i = 0; i < session->torrents_count; i++)
    {
        printf("%d, %s\n", argc, argv[0]);
        printf("Torrent number %d\n", i+1);
        printf("\tName: %s\n", session->torrents[i]->info->name);
        printf("\tCreated by: %s\n", session->torrents[i]->info->created_by);
        printf("\tComment: %s\n", session->torrents[i]->info->comment);
        printf("\tCreation date: %ld\n", session->torrents[i]->info->creation_date);
        printf("\tPieces length: %ld\n", session->torrents[i]->info->piece_length);
        printf("\tTotal size: %ld\n", session->torrents[i]->info->total_size);
        printf("\tAmount of files: %d\n", session->torrents[i]->info->files_count);
        printf("\tFiles:\n");
        for (j = 0; j < session->torrents[i]->info->files_count; j++)
        {
            printf("\t\tPath: %s\n", session->torrents[i]->info->files[j].path);
            printf("\t\tLength: %ld\n", session->torrents[i]->info->files[j].length);
            printf("\t\tFirst piece: %d\n", session->torrents[i]->info->files[j].first_piece);
            printf("\t\tLast piece: %d\n", session->torrents[i]->info->files[j].last_piece);
            printf("\t\tOffset: %ld\n", session->torrents[i]->info->files[j].offset);
        }

        printf("\tTrackers:\n");
        for (j = 0; j < session->torrents[i]->info->trackers_length; j++)
            printf("\t\t%s\n", session->torrents[i]->info->trackers[j].announce);
        printf("\tHash info:");
        for(int j = 0; j < 20; j++) {
            printf("%02x", session->torrents[i]->info->info_hash[j]);
        };
        printf("\n");
        printf("\tDownload path: %s\n", session->torrents[i]->download_path);
    }
}
// void CMD_delete(CL_session *session, int argc, char **argv)
// {
//     return;
// }
// void CMD_connect(CL_session *session, int argc, char **argv)
// {
//     return;
// }
// void CMD_help(CL_session *session, int argc, char **argv)
// {
//     return;
// }

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
