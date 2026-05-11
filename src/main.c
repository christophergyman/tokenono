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

    const float SQUARE_SIZE = 100.0f;
    const float PLAYER_SPEED = 240.0f;
    Vector2 square_position = {
        .x = SCREEN_WIDTH / 2.0f - SQUARE_SIZE / 2.0f,
        .y = SCREEN_HEIGHT / 2.0f - SQUARE_SIZE / 2.0f,
    };
    const Vector2 square_size = {
        .x = SQUARE_SIZE,
        .y = SQUARE_SIZE,
    };

    while (!WindowShouldClose()) {
        // Anything allocated from frame_arena must not survive past this frame.
        arena_reset(&frame_arena);

        Vector2 move_direction = {0};
        if (IsKeyDown(KEY_W)) {
            move_direction.y -= 1.0f;
        }
        if (IsKeyDown(KEY_S)) {
            move_direction.y += 1.0f;
        }
        if (IsKeyDown(KEY_A)) {
            move_direction.x -= 1.0f;
        }
        if (IsKeyDown(KEY_D)) {
            move_direction.x += 1.0f;
        }

        if (move_direction.x != 0.0f && move_direction.y != 0.0f) {
            const float DIAGONAL_NORMALIZER = 0.70710678118f;
            move_direction.x *= DIAGONAL_NORMALIZER;
            move_direction.y *= DIAGONAL_NORMALIZER;
        }

        const float delta_time = GetFrameTime();
        square_position.x += move_direction.x * PLAYER_SPEED * delta_time;
        square_position.y += move_direction.y * PLAYER_SPEED * delta_time;

        BeginDrawing();
        ClearBackground(RAYWHITE);
        DrawRectangleV(square_position, square_size, DARKBLUE);
        EndDrawing();
    }

    CloseWindow();

    // Destroy arenas after the game loop; this releases their backing buffers.
    arena_destroy(&frame_arena);
    arena_destroy(&permanent_arena);

    return 0;
}
