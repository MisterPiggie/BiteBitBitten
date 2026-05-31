#include "pool_thread.h"



void *worker(void *arg)
{
    CL_threadpool *pool = arg;
    CL_task task;
     
    while (1)
    {
        pthread_mutex_lock(&pool->lock);

        while (pool->count == 0 && !pool->stop)
            pthread_cond_wait(&pool->cond, &pool->lock);

        if (pool->stop)
        {
            pthread_mutex_unlock(&pool->lock);
            return NULL;
        }

        task = pool->queue[pool->head];
        pool->head = (pool->head + 1) % 64;
        pool->count--;

        pthread_mutex_unlock(&pool->lock);

        task.fn(task.arg);

    }
    return NULL;
}

void threadpool_init(CL_threadpool *pool)
{
    pool->head = 0;
    pool->tail = 0;
    pool->count = 0;
    pool->stop = false;

    pthread_mutex_init(&pool->lock, NULL);
    pthread_cond_init(&pool->cond, NULL);

    for (int i = 0; i < 4; i++)
        pthread_create(&pool->threads[i], NULL, worker, pool);

}
void threadpool_push(CL_threadpool *pool, task_fn fn, void *arg)
{
    CL_task task = {fn, arg};

    pthread_mutex_lock(&pool->lock);
    pool->queue[pool->tail] = task;
    pool->tail = (pool->tail + 1) % 64;
    pool->count++;

    pthread_cond_signal(&pool->cond);
    pthread_mutex_unlock(&pool->lock);
}

void threadpool_destroy(CL_threadpool *pool)
{
    pthread_mutex_lock(&pool->lock);
    pool->stop = true;
    pthread_cond_broadcast(&pool->cond);
    pthread_mutex_unlock(&pool->lock);

    for (int i = 0; i < 4; i++)
        pthread_join(pool->threads[i], NULL);
}
