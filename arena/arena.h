#ifndef STB_ARENA_H
#include <stddef.h>
#include <stdint.h>

#define KB(x) ((size_t)(x) * 1024)
#define MB(x) (KB(x) * 1024)
#define GB(x) (MB(x) * 1024)


#define COMMIT_CHUNK KB(64)


typedef struct
{
    uint8_t *memory;
    size_t  used;
    size_t  commited;
    size_t  reserved;
} Arena;

Arena arena_create(size_t reserve_size);
void *arena_push(Arena *a, size_t size);
void arena_reset(Arena *a);
void arena_rewind(Arena *a);
void arena_destroy(Arena *a);

#define push_array(arena, type, count)  (type *)arena_push((arena), sizeof(type)*(count))
#define push_struct(arena, type) push_array((arena), (type), 1)

#endif // !STB_ARENA_H

#ifdef ARENA_IMPLEMENTATION

#include <sys/mman.h>



static void *os_reserve(size_t size)
{
    void *ptr = mmap(NULL, size, PROT_NONE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    return ptr == MAP_FAILED ? NULL : ptr;
}

static void os_commit(void *ptr, size_t size)
{
    mprotect(ptr, size, PROT_READ | PROT_WRITE);
}


static void os_release(void *ptr, size_t size)
{
    munmap(ptr, size);
}

Arena arena_create(size_t reserve_size)
{
    Arena a = {0};
    a.memory = (uint8_t *)os_reserve(reserve_size);
    a.reserved = reserve_size;
    return a;
}

void *arena_push(Arena *a, size_t size)
{
    size_t new_commit;
    void *ptr;

    if (a->used + size > a->commited) 
    {
        new_commit = a->commited + COMMIT_CHUNK;
        assert(a->used + size <= a->reserved && "Arena out of reserved space");
        os_commit(a->memory, new_commit);
        a->commited = new_commit;
    }

    ptr = a->memory + a->used;
    a->used += size;

    return ptr;

}

void arena_rewind(Arena *a)
{
    a->used = 0;
}

void arena_reset(Arena *a)
{
    a->used = 0;
    os_commit(a->memory, a->used);
}
void arena_destroy(Arena *a)
{
    os_release(a->memory, a->reserved);
    *a = (Arena){0};
}
#endif // ARENA_IMPLEMENTATION
