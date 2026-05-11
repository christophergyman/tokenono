#include <raylib.h>

#include <stdbool.h>

#include "app_config.h"
#include "arena.h"
#include "arena_tests.h"

static float clamp_float(float value, float min, float max)
{
    if (value < min) {
        return min;
    }
    if (value > max) {
        return max;
    }
    return value;
}

static float move_toward_zero(float value, float amount)
{
    if (value > 0.0f) {
        value -= amount;
        return value < 0.0f ? 0.0f : value;
    }

    if (value < 0.0f) {
        value += amount;
        return value > 0.0f ? 0.0f : value;
    }

    return 0.0f;
}

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
    const float FLOOR_Y = SCREEN_HEIGHT - SQUARE_SIZE;
    const float MOVE_ACCELERATION = 2200.0f;
    const float MAX_RUN_SPEED = 360.0f;
    const float GROUND_FRICTION = 5200.0f;
    const float AIR_FRICTION = 1200.0f;
    const float GRAVITY = 1800.0f;
    const float JUMP_VELOCITY = -650.0f;
    const float FAST_FALL_ACCELERATION = 1200.0f;

    Vector2 square_position = {
        .x = SCREEN_WIDTH / 2.0f - SQUARE_SIZE / 2.0f,
        .y = FLOOR_Y,
    };
    Vector2 player_velocity = {0};
    bool is_grounded = true;

    const Vector2 square_size = {
        .x = SQUARE_SIZE,
        .y = SQUARE_SIZE,
    };

    while (!WindowShouldClose()) {
        // Anything allocated from frame_arena must not survive past this frame.
        arena_reset(&frame_arena);

        const float delta_time = GetFrameTime();

        float input_x = 0.0f;
        if (IsKeyDown(KEY_A)) {
            input_x -= 1.0f;
        }
        if (IsKeyDown(KEY_D)) {
            input_x += 1.0f;
        }

        if (input_x != 0.0f) {
            player_velocity.x += input_x * MOVE_ACCELERATION * delta_time;
            player_velocity.x = clamp_float(player_velocity.x, -MAX_RUN_SPEED, MAX_RUN_SPEED);
        } else {
            const float friction = is_grounded ? GROUND_FRICTION : AIR_FRICTION;
            player_velocity.x = move_toward_zero(player_velocity.x, friction * delta_time);
        }

        if (IsKeyPressed(KEY_W) && is_grounded) {
            player_velocity.y = JUMP_VELOCITY;
            is_grounded = false;
        }

        player_velocity.y += GRAVITY * delta_time;

        if (IsKeyDown(KEY_S) && !is_grounded) {
            player_velocity.y += FAST_FALL_ACCELERATION * delta_time;
        }

        square_position.x += player_velocity.x * delta_time;
        square_position.y += player_velocity.y * delta_time;

        if (square_position.y >= FLOOR_Y) {
            square_position.y = FLOOR_Y;
            player_velocity.y = 0.0f;
            is_grounded = true;
        } else {
            is_grounded = false;
        }

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
