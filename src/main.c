#include <raylib.h>

#include <math.h>
#include <stdbool.h>

#include "app_config.h"
#include "arena.h"
#include "arena_tests.h"

#define TILE_SIZE 32
#define MAP_WIDTH 25
#define MAP_HEIGHT 19

static const char *LEVEL_MAP[MAP_HEIGHT] = {
    ".........................",
    ".........................",
    ".........................",
    ".........................",
    "..................###....",
    ".........................",
    "............###..........",
    ".........................",
    "......###................",
    ".........................",
    ".........................",
    ".....................####",
    ".........................",
    ".........................",
    "###......................",
    ".........................",
    ".........................",
    ".........................",
    "#########################",
};

typedef struct Player {
    Vector2 position;
    Vector2 velocity;
    bool is_grounded;
    int max_health;
    int health;
} Player;

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

static int clamp_int(int value, int min, int max)
{
    if (value < min) {
        return min;
    }
    if (value > max) {
        return max;
    }
    return value;
}

static bool is_solid_tile(int tile_x, int tile_y)
{
    if (tile_x < 0 || tile_x >= MAP_WIDTH || tile_y < 0 || tile_y >= MAP_HEIGHT) {
        return false;
    }

    return LEVEL_MAP[tile_y][tile_x] == '#';
}

static int world_to_tile(float world_position)
{
    return (int)floorf(world_position / TILE_SIZE);
}

static bool player_hits_horizontal_tile(Vector2 position, Vector2 size, int tile_x)
{
    const int top_tile = world_to_tile(position.y);
    const int bottom_tile = world_to_tile(position.y + size.y - 0.001f);

    for (int tile_y = top_tile; tile_y <= bottom_tile; tile_y++) {
        if (is_solid_tile(tile_x, tile_y)) {
            return true;
        }
    }

    return false;
}

static bool player_hits_vertical_tile(Vector2 position, Vector2 size, int tile_y)
{
    const int left_tile = world_to_tile(position.x);
    const int right_tile = world_to_tile(position.x + size.x - 0.001f);

    for (int tile_x = left_tile; tile_x <= right_tile; tile_x++) {
        if (is_solid_tile(tile_x, tile_y)) {
            return true;
        }
    }

    return false;
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

    const float PLAYER_SIZE = 28.0f;
    const float MOVE_ACCELERATION = 1500.0f;
    const float MAX_RUN_SPEED = 240.0f;
    const float GROUND_FRICTION = 3600.0f;
    const float AIR_FRICTION = 800.0f;
    const float GRAVITY = 1200.0f;
    const float JUMP_VELOCITY = -430.0f;
    const float FAST_FALL_ACCELERATION = 800.0f;

    Player player = {
        .position = {
            .x = 64.0f,
            .y = 18.0f * TILE_SIZE - PLAYER_SIZE,
        },
        .velocity = {0},
        .is_grounded = true,
        .max_health = 100,
        .health = 100,
    };

    const Vector2 player_size = {
        .x = PLAYER_SIZE,
        .y = PLAYER_SIZE,
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
            player.velocity.x += input_x * MOVE_ACCELERATION * delta_time;
            player.velocity.x = clamp_float(player.velocity.x, -MAX_RUN_SPEED, MAX_RUN_SPEED);
        } else {
            const float friction = player.is_grounded ? GROUND_FRICTION : AIR_FRICTION;
            player.velocity.x = move_toward_zero(player.velocity.x, friction * delta_time);
        }

        if (IsKeyPressed(KEY_W) && player.is_grounded) {
            player.velocity.y = JUMP_VELOCITY;
            player.is_grounded = false;
        }

        if (IsKeyPressed(KEY_K)) {
            player.health = clamp_int(player.health - 10, 0, player.max_health);
        }
        if (IsKeyPressed(KEY_L)) {
            player.health = clamp_int(player.health + 10, 0, player.max_health);
        }

        player.velocity.y += GRAVITY * delta_time;

        if (IsKeyDown(KEY_S) && !player.is_grounded) {
            player.velocity.y += FAST_FALL_ACCELERATION * delta_time;
        }

        player.position.x += player.velocity.x * delta_time;
        if (player.velocity.x > 0.0f) {
            const int right_tile = world_to_tile(player.position.x + player_size.x - 0.001f);
            if (player_hits_horizontal_tile(player.position, player_size, right_tile)) {
                player.position.x = right_tile * TILE_SIZE - player_size.x;
                player.velocity.x = 0.0f;
            }
        } else if (player.velocity.x < 0.0f) {
            const int left_tile = world_to_tile(player.position.x);
            if (player_hits_horizontal_tile(player.position, player_size, left_tile)) {
                player.position.x = (left_tile + 1) * TILE_SIZE;
                player.velocity.x = 0.0f;
            }
        }

        player.position.y += player.velocity.y * delta_time;
        player.is_grounded = false;
        if (player.velocity.y > 0.0f) {
            const int bottom_tile = world_to_tile(player.position.y + player_size.y - 0.001f);
            if (player_hits_vertical_tile(player.position, player_size, bottom_tile)) {
                player.position.y = bottom_tile * TILE_SIZE - player_size.y;
                player.velocity.y = 0.0f;
                player.is_grounded = true;
            }
        } else if (player.velocity.y < 0.0f) {
            const int top_tile = world_to_tile(player.position.y);
            if (player_hits_vertical_tile(player.position, player_size, top_tile)) {
                player.position.y = (top_tile + 1) * TILE_SIZE;
                player.velocity.y = 0.0f;
            }
        }

        BeginDrawing();
        ClearBackground(RAYWHITE);
        for (int tile_y = 0; tile_y < MAP_HEIGHT; tile_y++) {
            for (int tile_x = 0; tile_x < MAP_WIDTH; tile_x++) {
                if (is_solid_tile(tile_x, tile_y)) {
                    DrawRectangle(tile_x * TILE_SIZE,
                                  tile_y * TILE_SIZE,
                                  TILE_SIZE,
                                  TILE_SIZE,
                                  GRAY);
                    DrawRectangleLines(tile_x * TILE_SIZE,
                                       tile_y * TILE_SIZE,
                                       TILE_SIZE,
                                       TILE_SIZE,
                                       DARKGRAY);
                }
            }
        }
        DrawRectangleV(player.position, player_size, DARKBLUE);

        const int health_bar_x = 20;
        const int health_bar_y = 20;
        const int health_bar_width = 220;
        const int health_bar_height = 24;
        const float health_percent = (float)player.health / (float)player.max_health;
        const int current_health_width = (int)(health_bar_width * health_percent);

        DrawRectangle(health_bar_x, health_bar_y, health_bar_width, health_bar_height, DARKGRAY);
        DrawRectangle(health_bar_x, health_bar_y, current_health_width, health_bar_height, RED);
        DrawRectangleLines(health_bar_x, health_bar_y, health_bar_width, health_bar_height, BLACK);
        DrawText(TextFormat("HP %d/%d", player.health, player.max_health),
                 health_bar_x,
                 health_bar_y + health_bar_height + 6,
                 16,
                 BLACK);
        DrawText("K damage | L heal", health_bar_x, health_bar_y + health_bar_height + 26, 14, DARKGRAY);

        EndDrawing();
    }

    CloseWindow();

    // Destroy arenas after the game loop; this releases their backing buffers.
    arena_destroy(&frame_arena);
    arena_destroy(&permanent_arena);

    return 0;
}
