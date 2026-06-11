#include "pool_thread.h"
#include "../announcer/announcer.h"
#include "../parser/parser.h"
#include "../SHA1/SHA1.h"

#include <netdb.h>
#include <curl/curl.h>
#include <unistd.h>
#include <fcntl.h>

void task_resolve_DNS(void *arg)
{
    char port_str[6];
    DNS_resolve_args args = *(DNS_resolve_args *)arg;

    DNS_resolve_result result = 
    {
        .torrent = args.torrent,
        .tracker = args.tracker,
        .success = false,
        .ip      = 0,
    };

    struct addrinfo hints = {0};
    struct addrinfo *res;

    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_DGRAM;

    snprintf(port_str, sizeof(port_str), "%u", args.tracker->port);

    if (getaddrinfo(args.tracker->host, port_str, &hints, &res) == 0)
    {
        result.ip = ((struct sockaddr_in *) res->ai_addr)->sin_addr.s_addr;
        result.success = true;
        freeaddrinfo(res);
    }
    
    notify_result notify = {
        .type = NOTIFY_DNS_RESOLVE,
        .dns = result,
    };

    write(args.pipefd, &notify, sizeof(notify));
}

void notify_pipe_readable(EV_loop *loop)
{
    notify_result result;
    read(loop->notify_pipe[0], &result, sizeof(result));

    switch (result.type)
    {
        case NOTIFY_DNS_RESOLVE:
            handle_dns_result(loop, &result.dns);
            break;

        case NOTIFY_HTTP_RESPONSE:
            handle_HTTP_response(loop, &result.http);
            break;

        case NOTIFY_SHA1_VERIFY:
            // handle_sha1_result(loop, &result.sha1);
            break;

        case NOTIFY_DISK_WRITE:
            // handle_disk_result(loop, &result.disk);
            break;
    }
}

void handle_dns_result(EV_loop *loop, DNS_resolve_result *result)
{
    (void) loop;
    if (!result->success)
    {
        result->tracker->state = TRACKER_FAILED;
        return;
    }

    result->tracker->ip = result->ip;
    result->tracker->state = TRACKER_IDLE;
}

void task_HTTP_announce(void *arg)
{
    char url[2048];
    char response[2048] = {0};
    char announce[1024];
    char encoded_hash[61];

    HTTP_announce_args args = *(HTTP_announce_args *)arg;
    CURL_buf cb = {.buf = (uint8_t *)response, .len = 0};

    notify_result result =
    {
        .type = NOTIFY_HTTP_RESPONSE,
        .http =
        {
            .tracker = args.tracker,
            .torrent = args.torrent,
            .success = false,
            .interval = 1800,
        }
    };

    HTTP_url_encode_hash(args.torrent->info->info_hash, encoded_hash);
    snprintf(announce, sizeof(announce), "%s://%s:%u/%s", args.tracker->schema, args.tracker->host, args.tracker->port, args.tracker->path);

    snprintf(url, sizeof(url),
            "%s?info_hash=%s&peer_id=%s&port=%d&uploaded=%llu&downloaded=%llu&left=%llu&compact=1&numwant=50&event=%s",
            announce, encoded_hash, args.session->peer_id, DEFAULT_PORT, 
            (unsigned long long) args.torrent->uploaded,
            (unsigned long long) args.torrent->downloaded, 
            (unsigned long long) args.torrent->info->total_size - args.torrent->downloaded,
            args.torrent->state == TORRENT_DOWNLOADING ? "started" : "completed");

    CURL *curl = curl_easy_init();
    curl_easy_setopt(curl, CURLOPT_URL,           url);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, curl_write_cb);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA,     &cb);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT,       15L);

    CURLcode res = curl_easy_perform(curl);
    curl_easy_cleanup(curl);

    if (res != CURLE_OK)
    {
        write(args.pipefd, &result, sizeof(result));
        return;
    }


    Arena scratch_arena = arena_create(KB(128));

    file_content_buffer buffer = 
    {
        .data = (uint8_t *)response,
        .size = cb.len,
    };
    
    BEN_parser *parser = init_BEN_parser(&scratch_arena, &buffer);

    BEN_value *dict = parse_dict(&scratch_arena, parser);
    if (!dict)
    {
        arena_destroy(&scratch_arena);
        write(args.pipefd, &result, sizeof(result));
        return;
    }

    BEN_value *failure = get_BEN_value_by_key(dict->dict, "failure reason");
    if (failure)
    {
        arena_destroy(&scratch_arena);
        write(args.pipefd, &result, sizeof(result));
        return;
    }

    BEN_value *interval = get_BEN_value_by_key(dict->dict, "interval");
    BEN_value *peers = get_BEN_value_by_key(dict->dict, "peers");

    result.http.success  = true;
    result.http.interval = interval ? interval->number : 1800;

    if (peers)
    {
        result.http.peers_len = peers->string.length;
        memcpy(result.http.peers, peers->string.data, peers->string.length);
    }

    arena_destroy(&scratch_arena);

    write(args.pipefd, &result, sizeof(result));
            
}

size_t curl_write_cb(void *data, size_t size, size_t nmemb, void *userp)
{
    size_t real_size = size * nmemb;
    CURL_buf *cb = userp;

    if (cb->len + real_size >= 2048)
        return 0;  

    memcpy(cb->buf + cb->len, data, real_size);
    cb->len += real_size;
    return real_size;
}

void handle_HTTP_response(EV_loop *loop, HTTP_announce_result *result)
{
    (void) loop;
    if (!result->success)
    {
        result->tracker->state = TRACKER_FAILED;
        return;
    }

    result->tracker->state         = TRACKER_ALIVE;
    result->tracker->interval      = result->interval;
    result->tracker->last_announce = time(NULL);

    parse_peers(result->peers, result->peers_len, result->torrent);
}

void task_sha1_verify(void *arg)
{
    piece_task task = *(piece_task *)arg;
    uint8_t hash[20];

    SHA1_hash(task.data, task.data_len, hash);

    uint8_t *expected = task.torrent->info->pieces + (task.piece_idx * 20);

    notify_result result = {
        .type = NOTIFY_SHA1_VERIFY,
        .sha1 = {
            .torrent   = task.torrent,
            .peer      = task.peer,
            .piece_idx = task.piece_idx,
            .success   = memcmp(hash, expected, 20) == 0,
        }
    };

    write(task.pipefd, &result, sizeof(result));
}

void handle_sha1_result(EV_loop *loop, SHA1_verify_result *result)
{
    TR_torrent *tr   = result->torrent;
    TR_peer    *peer = result->peer;

    if (!result->success)
    {
        tr->requested_pieces[peer->current_piece / 8] &= ~(1 << (7 - peer->current_piece % 8));
        peer->failed_tries++;
        assign_piece_to_peer(peer, tr);
        return;
    }

    piece_task *task = arena_push_struct(&tr->arena, piece_task);
    task->torrent   = tr;
    task->peer      = peer;
    task->piece_idx = result->piece_idx;
    task->data_len  = tr->info->pieces_count;
    task->pipefd    = loop->notify_pipe[1];
    task->data      = peer->piece_buf;

    threadpool_push(loop->pool, task_disk_write, task);
}

void task_disk_write(void *arg)
{
    piece_task task = *(piece_task *)arg;

    notify_result result = {
        .type = NOTIFY_DISK_WRITE,
        .disk = {
            .torrent   = task.torrent,
            .peer      = task.peer,
            .piece_idx = task.piece_idx,
            .success   = false,
        }
    };

    if (write_piece_to_disk(task.torrent, task.piece_idx, task.data, task.data_len))
        result.disk.success = true;

    write(task.pipefd, &result, sizeof(result));
}

bool write_piece_to_disk(TR_torrent *tr, uint32_t piece_idx, uint8_t *data, uint32_t data_len)
{
    uint64_t piece_offset = (uint64_t)piece_idx * tr->info->piece_length;
    uint32_t written      = 0;

    for (int i = 0; i < tr->info->files_count; i++)
    {
        TR_file *file = &tr->info->files[i];

        if (piece_offset + data_len <= file->offset)          
            break;
        if (file->offset + file->length <= piece_offset)   
            continue;

        uint64_t file_start  = piece_offset > file->offset ? piece_offset - file->offset : 0;
        uint32_t to_write    = file->length - file_start;
        if (to_write > data_len - written)
            to_write = data_len - written;

        int fd = open(file->path, O_WRONLY);
        if (fd < 0) 
            return false;

        ssize_t n = pwrite(fd, data + written, to_write, file_start);
        close(fd);

        if (n != (ssize_t)to_write)
            return false;

        written += to_write;
        if (written == data_len) 
            break;
    }

    return true;
}

void handle_disk_result(EV_loop *loop, disk_write_result *result)
{
    TR_torrent *tr = result->torrent;
    int i          = result->piece_idx;

    if (!result->success)
    {
        tr->requested_pieces[i / 8] &= ~(1 << (7 - i % 8));
        assign_piece_to_peer(result->peer, tr);
        return;
    }

    tr->bitfield[i / 8] |= (1 << (7 - i % 8));
    tr->downloaded      += tr->info->piece_length;

    for (int j = 0; j < tr->swarm->peers_count; j++)
        send_have(tr->swarm->peers[j], i);

    assign_piece_to_peer(result->peer, tr);
}
