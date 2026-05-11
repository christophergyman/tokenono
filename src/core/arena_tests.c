#include "arena_tests.h"

#include "arena.h"

#include <stdalign.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

// Mixed fields make the smoke test cover zeroing and normal C alignment needs.
typedef struct SmokeValue {
    int value;
    double weight;
} SmokeValue;

static void smoke_check(int condition, const char *message)
{
    if (!condition) {
        fprintf(stderr, "arena smoke test failed: %s\n", message);
        abort();
    }
}

// Arena allocations should be safe to cast to regular C structs without alignment footguns.
static void check_aligned(void *ptr)
{
    smoke_check(((uintptr_t)ptr % alignof(max_align_t)) == 0, "allocation is not max_align_t aligned");
}

// This intentionally avoids OOM tests because OOM correctly aborts the process.
void run_arena_smoke_test(void)
{
    Arena test_arena;
    arena_init(&test_arena, KILOBYTES(1));

    SmokeValue *first = arena_alloc(&test_arena, sizeof(*first));
    check_aligned(first);
    smoke_check(first->value == 0, "first allocation int field was not zeroed");
    smoke_check(first->weight == 0.0, "first allocation double field was not zeroed");

    first->value = 42;
    first->weight = 1.5;

    unsigned char *expected_reuse = test_arena.base;
    arena_reset(&test_arena);

    // After reset, the next allocation should reuse the start and be zeroed again.
    SmokeValue *second = arena_alloc(&test_arena, sizeof(*second));
    check_aligned(second);
    smoke_check((unsigned char *)second == expected_reuse, "reset did not reuse arena start");
    smoke_check(second->value == 0, "second allocation int field was not zeroed");
    smoke_check(second->weight == 0.0, "second allocation double field was not zeroed");

    arena_destroy(&test_arena);

    printf("arena smoke test passed\n");
}
