#include <raylib.h>

#include "app_config.h"
#include "arena.h"
#include "arena_tests.h"

int main(int argc, char **argv)
{
    const AppConfig config = app_config_parse(argc, argv);

    // Keep allocator checks close to startup so memory bugs fail before gameplay runs.
    if (config.debug_enabled) {
        run_arena_smoke_test();
    }

    const size_t PERMANENT_ARENA_SIZE = MEGABYTES(8);
    const size_t FRAME_ARENA_SIZE = MEGABYTES(1);

    // Permanent memory lives for the whole app; frame memory is reset every frame.
    Arena permanent_arena;
    Arena frame_arena;

    arena_init(&permanent_arena, PERMANENT_ARENA_SIZE);
    arena_init(&frame_arena, FRAME_ARENA_SIZE);

    const int SCREEN_WIDTH = 800;
    const int SCREEN_HEIGHT = 600;

    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "square game");
    SetTargetFPS(60);

    const int SQUARE_SIZE = 100;
    const int SQUARE_X = SCREEN_WIDTH / 2 - SQUARE_SIZE / 2;
    const int SQUARE_Y = SCREEN_HEIGHT / 2 - SQUARE_SIZE / 2;

    while (!WindowShouldClose()) {
        // Anything allocated from frame_arena must not survive past this frame.
        arena_reset(&frame_arena);

        BeginDrawing();
        ClearBackground(RAYWHITE);
        DrawRectangle(SQUARE_X, SQUARE_Y, SQUARE_SIZE, SQUARE_SIZE, DARKBLUE);
        EndDrawing();
    }

    CloseWindow();

    // Destroy arenas after the game loop; this releases their backing buffers.
    arena_destroy(&frame_arena);
    arena_destroy(&permanent_arena);

    return 0;
}
