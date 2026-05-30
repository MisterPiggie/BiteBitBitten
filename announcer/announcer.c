#include "announcer.h"
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h>
#include <stdio.h>
#include <time.h>
#include <sys/random.h>


void ANN_announcer_tick(EV_loop *loop, TR_torrent *tr)
{
    NET_tracker *tracker = &tr->tracks[tr->active_tracker_idx];
    switch (tracker->state)
    {
        case TRACKER_IDLE:
            ANN_make_announce_req(tr, loop);
            tr->track_state = TRACKER_ANNOUNCING;
            tracker->announce_sent_at = time(NULL);
            break;
        case TRACKER_ANNOUNCING:
            if (time(NULL) - tracker->announce_sent_at > 15)
            {
                tracker->state = TRACKER_FAILED;
                if (++tr->active_tracker_idx >= tr->tracker_count)
                    tr->state = TORRENT_FAILED;
            }
            break;
        case TRACKER_ALIVE:
            if (time(NULL) - tracker->last_announce > tracker->interval)
            {
                ANN_make_announce_req(tr, loop);
                tr->track_state = TRACKER_ANNOUNCING;
                tracker->announce_sent_at = time(NULL);
            }
            break;
        case TRACKER_FAILED:
            if (++tr->active_tracker_idx >= tr->tracker_count)
                tr->state = TORRENT_FAILED;
            break;
    }
}


void ANN_make_announce_req(TR_torrent *tr, EV_loop *loop)
{
    NET_tracker *tracker = &tr->tracks[tr->active_tracker_idx];
    if (strcmp(tracker->schema, "https") == 0 ||
        strcmp(tracker->schema, "http") == 0)
    {
        make_HTTP_announce(tr, loop->session->peer_id);
        return;
    }

    if (strcmp(tracker->schema, "udp") == 0)
    {
        make_UDP_connect(tr, loop->session->peer_id);
        return;
    }

    return;
}

void generate_peer_id(unsigned char peer_id[20])
{
   unsigned char buf[12];
   int i;
   for (i = 0; i < peer_id_version_len; i++)
   {
       peer_id[i] = peer_id_version[i];
   }

   if (getrandom(buf, sizeof(buf), 0) == sizeof(buf))
   {
       for (i = 0; i < (int) sizeof(buf); i++)
       {
           peer_id[i + peer_id_version_len] = charset[buf[i] % charset_len];
       }

       return;
   }

   FILE *f = fopen("/dev/urandom", "rb");
   if (f) 
   {
       if (fread(buf, sizeof(unsigned char), sizeof(buf), f) == 12)
       {
           for (i = 0; i < (int) sizeof(buf); i++)
           {
               peer_id[i + peer_id_version_len] = charset[buf[i] % charset_len];
           }
           fclose(f);
       }

       fclose(f);
   }
}

void url_encode_hash(const uint8_t *hash_info, char *out)
{
    //OUT MUST BE 61 CHARS LONG
    int i;
    for (i = 0; i < 20; i++)
    {
        sprintf(out + i*3, "%%%02X", hash_info[i]);
    }
    out[60] = '\0';
}

uint32_t get_random_u32(void)
{
    uint32_t x;

    if (getrandom(&x, sizeof(x), 0) == sizeof(x))
        return x;

    FILE *f = fopen("/dev/urandom", "rb");
    if (f) 
    {
        if (fread(&x, sizeof(x), 1, f) == 1)
        {
            fclose(f);
            return x;
        }

        fclose(f);
    }
    return (uint32_t)(time(NULL) ^ getpid() ^ (uintptr_t)&x);
}

 

int tracker_string_to_NET_tracker(Arena *arena, char *url, NET_tracker *track)
{
    char *schema_end;
    char *host_start;
    char *path_start;
    char *colon;

    schema_end = strstr(url, "://");
    if (!schema_end)
        return -1;

    track->schema = arena_push_strn(arena, url, schema_end - url);
    host_start = schema_end + 3;

    colon = strchr(host_start, ':');
    if (!colon)
        return -1;

    track->host = arena_push_strn(arena, host_start, colon - host_start);

    track->port = atoi(colon + 1);

    path_start = strchr(colon + 1, '/');
    track->path = path_start ? arena_push_str(arena, path_start) : arena_push_str(arena, "/");
   
    track->state =TRACKER_NOT_RESOLVED;


    return 0;
}

