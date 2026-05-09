#include <endian.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <time.h>
#include "types/types.h"
#include "announcer/announcer.h"
#include "files/file_interactions.h"
#include "parser/parser.h"
#include "init/init_session.h"
#include "utils/str_utils.h"

int main(int argc, char **argv)
{
    int i;
    file_content_buffer buffer;
    BEN_value *top_dict;
    BEN_parser parser;
    TR_info *info = malloc(sizeof(TR_info));
    uint8_t hash[20];
    unsigned char peer_id[20];
    memset(hash, 0, sizeof(hash));
    CL_session *session = malloc(sizeof(CL_session)); 

    if (argc != 2) 
    {
        fprintf(stderr, "ERROR: no file provided\n");
        exit(EXIT_FAILURE);
    }
    
    
    buffer = read_BEN_file(argv[1]);
    if (buffer.size == 0)
    {
        fprintf(stderr, "ERROR: file couldnt be open\n");
        exit(EXIT_FAILURE);
    }

    init_CL_session(session);



    printf("Config dir path: %s\n", session->config_dir_path);
    printf("Config file path: %s\n", session->config_file_path);
    printf("Resume path: %s\n", session->resume_dir_path);
    printf("Torrent path: %s\n", session->torrent_dir_path);

    make_dir_recursive(session->config_dir_path, 0755);
    make_dir_recursive(session->resume_dir_path, 0755);
    make_dir_recursive(session->torrent_dir_path, 0755);

    init_BEN_parser(buffer, &parser);
    top_dict = parse_file_content_buffer(&parser);

    if (top_dict->dict.count == 0)
    {
        fprintf(stderr, "ERROR: file couldnt be parsed\n");
        exit(EXIT_FAILURE);
    }



    BEN_pairs_to_TR_info(&top_dict->dict, info);
    fill_in_calculated_field_in_TR_info(info);
    printf("Name: %s\n", info->name);
    printf("Created by: %s\n", info->created_by);
    printf("Creation date: %ld\n", info->creation_date);
    printf("Trackers: \n");
    for (int i = 0; i < info->trackers_length; i++)
    {
        trim_spaces(info->trackers[i].announce);
        printf("    %s\n", info->trackers[i].announce);
    }
    printf("Amount of files: %d\n", info->files_count);
    for (int i = 0; i < info->files_count; i++)
    {
        printf("    Filepath: %s\n", info->files[i].path);
        printf("    File size: %ld\n", info->files[i].length);
        printf("    Offset: %ld\n", info->files[i].offset);
        printf("    First piece: %d\n", info->files[i].first_piece);
        printf("    Last piece: %d\n", info->files[i].last_piece);
    }


    printf("Total size: %ld\n", info->total_size);

    get_info_hash(&parser, info->info_hash);
    printf("Hash: ");
    for(int i = 0; i < 20; i++) {
        printf("%02x", info->info_hash[i]);
    }
    printf("\n");

    generate_peer_id(peer_id); 
    printf("Peer ID: ");
    for(int i = 0; i < 20; i++) {
        printf("%c", peer_id[i]);
    }
    printf("\n");

    printf("\n");
    printf("Parsing successful\n");

    printf("Starting NET connections\n");

    NET_tracker *tracks = calloc( info->trackers_length, sizeof(NET_tracker));

    for (i = 0; i < info->trackers_length; i++)
    {
        tracker_string_to_NET_tracker(info->trackers[i].announce, &tracks[i]);
        printf("Schema: %s\n", tracks[i].schema);
        printf("Host: %s\n", tracks[i].host);
        printf("Path: %s\n", tracks[i].path);
        printf("Port: %d\n", tracks[i].port);
    }

    struct sockaddr_in addr;
    UDP_resolve_tracker(&tracks[1], &addr);

    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    UDP_send_connect_req(sock, (struct sockaddr *)&addr);

    UDP_resp_connect_packed resp;

    recvfrom(sock, &resp, sizeof(resp), 0, NULL, NULL);

    printf("Action: %d\n", be16toh(resp.action));
    printf("connection_id: %u\n", be32toh(resp.connection_id));
    printf("transction_id: %d\n", be32toh(resp.transction_id));

}
