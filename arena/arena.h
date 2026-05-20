#ifndef STB_ARENA_H
#include <stddef.h>
#include <stdint.h>

#define KB(x) ((size_t)(x) * 1024)
#define MB(x) (KB(x) * 1024)
#define GB(x) (MB(x) * 1024)

typedef struct
{
    uint8_t *memory;
    size_t  used;
    size_t  commited;
    size_t  reserved;
} Arena;

Arena arena_create(size_t reserve_size);

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
#endif // ARENA_IMPLEMENTATION
