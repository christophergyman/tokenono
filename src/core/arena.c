#include "arena.h"

#include <stdalign.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// All arena errors are fatal by design so memory lifetime bugs fail loudly.
static void arena_abort(const char *file, int line, const char *message)
{
    fprintf(stderr, "arena error at %s:%d: %s\n", file, line, message);
    abort();
}

// Move the bump cursor to the next address safe for normal C types.
static size_t align_used(size_t used, size_t alignment, const char *file, int line)
{
    size_t padding = alignment - 1;

    if (used > SIZE_MAX - padding) {
        arena_abort(file, line, "alignment overflow");
    }

    return (used + padding) & ~padding;
}

// Allocate the arena backing buffer once; individual arena allocations come from this block.
void arena_init_debug(Arena *arena, size_t capacity, const char *file, int line)
{
    if (arena == NULL) {
        arena_abort(file, line, "arena pointer is NULL during init");
    }

    if (capacity == 0) {
        arena_abort(file, line, "capacity must be greater than zero");
    }

    arena->base = malloc(capacity);
    if (arena->base == NULL) {
        fprintf(stderr, "arena init failed at %s:%d: capacity=%zu\n", file, line, capacity);
        abort();
    }

    arena->capacity = capacity;
    arena->used = 0;
}

// Return zeroed memory from the arena and abort if the request cannot fit safely.
void *arena_alloc_debug(Arena *arena, size_t size, const char *file, int line)
{
    if (arena == NULL) {
        arena_abort(file, line, "arena pointer is NULL during allocation");
    }

    if (arena->base == NULL) {
        arena_abort(file, line, "arena is not initialized during allocation");
    }

    if (size == 0) {
        arena_abort(file, line, "allocation size must be greater than zero");
    }

    size_t aligned_used = align_used(arena->used, alignof(max_align_t), file, line);

    // Check before adding so a huge request cannot wrap around and appear to fit.
    if (size > SIZE_MAX - aligned_used) {
        arena_abort(file, line, "allocation size overflow");
    }

    size_t new_used = aligned_used + size;
    if (new_used > arena->capacity) {
        fprintf(stderr,
                "arena allocation failed at %s:%d: requested=%zu used=%zu aligned_used=%zu capacity=%zu\n",
                file,
                line,
                size,
                arena->used,
                aligned_used,
                arena->capacity);
        abort();
    }

    void *ptr = arena->base + aligned_used;
    memset(ptr, 0, size);
    arena->used = new_used;

    return ptr;
}

// Reset is intentionally cheap: old bytes remain, but future allocations overwrite from the start.
void arena_reset_debug(Arena *arena, const char *file, int line)
{
    if (arena == NULL) {
        arena_abort(file, line, "arena pointer is NULL during reset");
    }

    if (arena->base == NULL) {
        arena_abort(file, line, "arena is not initialized during reset");
    }

    arena->used = 0;
}

// Release the backing buffer and poison the metadata so double-destroy aborts later.
void arena_destroy_debug(Arena *arena, const char *file, int line)
{
    if (arena == NULL) {
        arena_abort(file, line, "arena pointer is NULL during destroy");
    }

    if (arena->base == NULL) {
        arena_abort(file, line, "arena is not initialized during destroy");
    }

    free(arena->base);
    arena->base = NULL;
    arena->capacity = 0;
    arena->used = 0;
}
