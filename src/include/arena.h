#ifndef ARENA_H
#define ARENA_H

#include <stddef.h>

#define KILOBYTES(n) ((size_t)(n) * 1024u)
#define MEGABYTES(n) (KILOBYTES(n) * 1024u)

// A linear arena owns one backing buffer and hands out zeroed memory by bumping `used` forward.
typedef struct Arena {
    unsigned char *base;
    size_t capacity;
    size_t used;
} Arena;

// Public calls are macros so allocator failures can report the original callsite.
#define arena_init(arena, capacity) \
    arena_init_debug((arena), (capacity), __FILE__, __LINE__)

#define arena_alloc(arena, size) \
    arena_alloc_debug((arena), (size), __FILE__, __LINE__)

#define arena_reset(arena) \
    arena_reset_debug((arena), __FILE__, __LINE__)

#define arena_destroy(arena) \
    arena_destroy_debug((arena), __FILE__, __LINE__)

// Internal implementations behind the callsite-capturing macros above.
void arena_init_debug(Arena *arena, size_t capacity, const char *file, int line);
void *arena_alloc_debug(Arena *arena, size_t size, const char *file, int line);
void arena_reset_debug(Arena *arena, const char *file, int line);
void arena_destroy_debug(Arena *arena, const char *file, int line);

#endif
