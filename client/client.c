#include "client.h"
#include "../announcer/announcer.h"


void CL_client_tick(EV_loop  *loop)
{
    int i;
    for (i = 0; i < loop->session->torrents_count; i++)
    {
        TR_torrent *tr = loop->session->torrents[i];

        switch (tr->state)
        {

            case TORRENT_DOWNLOADING:
                ANN_announcer_tick(loop, tr);
                ANN_refill_peers(loop, tr);
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
                break;
        }
    }
}
