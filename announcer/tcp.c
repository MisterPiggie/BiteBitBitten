#include "announcer.h"
#include "../threads/pool_thread.h"

void make_HTTP_announce(TR_torrent *tr, EV_loop *loop)
{
    HTTP_announce_args args = {
        .tracker = &tr->tracks[tr->active_tracker_idx],
        .torrent = tr,
        .session = loop->session,
        .pipefd = loop->notify_pipe[1],
    };

    threadpool_push(loop->pool, task_HTTP_announce, &args);

    tr->tracks[tr->active_tracker_idx].state            = TRACKER_ANNOUNCING;
    tr->tracks[tr->active_tracker_idx].announce_sent_at = time(NULL);
}

