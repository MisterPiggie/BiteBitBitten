#include "client.h"
#include "../announcer/announcer.h"


void CL_client_tick(EV_loop  *loop)
{
    int i, j;
    for (i = 0; i < loop->session->torrents_count; i++)
    {
        TR_torrent *tr = loop->session->torrents[i];

        switch (tr->state)
        {

            case TORRENT_DOWNLOADING:
                ANN_announcer_tick(loop, tr);
                // ANN_refill_peers(loop, tr);
                break;

            case TORRENT_NEEDS_CHECK:
                // FL_check_torrent(loop,tr);
                break;
            case TORRENT_CHECKING:
                break;
            case TORRENT_SEEDING:
                ANN_announcer_tick(loop, tr);
                break;
            case TORRENT_PAUSED:
                break;
            case TORRENT_FAILED:
                for (j = 0; j < tr->tracker_count; j++)
                if (strcmp(tr->tracks[j].schema, "udp") == 0)
                    tr->tracks[j]->state = TRACKER_NOT_RESOLVED;
       else if (strcmp(tr->tracks[j].schema, "http") || strcmp(tr->tracks[j].schema, "https"))
 tr->tracks[j]->state = TRACKER_IDLE;
        }
    }
}
