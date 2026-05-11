#include <raylib.h>

#include <math.h>
#include <stdbool.h>

#include "app_config.h"
#include "arena.h"
#include "arena_tests.h"
#include "ecs.h"

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

    EcsWorld *world = ecs_world_create(&permanent_arena, ECS_MAX_ENTITIES);

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

    const Entity player = ecs_create_entity(world);
    ecs_add_components(world,
                       player,
                       COMPONENT_POSITION |
                           COMPONENT_VELOCITY |
                           COMPONENT_COLLIDER |
                           COMPONENT_HEALTH |
                           COMPONENT_RENDER |
                           COMPONENT_PLAYER);

    world->positions[player].value = (Vector2){
        .x = 64.0f,
        .y = 18.0f * TILE_SIZE - PLAYER_SIZE,
    };
    world->velocities[player].value = (Vector2){0};
    world->colliders[player] = (ColliderComponent){
        .size = {
            .x = PLAYER_SIZE,
            .y = PLAYER_SIZE,
        },
        .is_grounded = true,
    };
    world->health[player] = (HealthComponent){
        .current = 100,
        .max = 100,
    };
    world->renders[player] = (RenderComponent){
        .size = world->colliders[player].size,
        .color = DARKBLUE,
    };

    while (!WindowShouldClose()) {
        // Anything allocated from frame_arena must not survive past this frame.
        arena_reset(&frame_arena);

        const float delta_time = GetFrameTime();
        PositionComponent *player_position = &world->positions[player];
        VelocityComponent *player_velocity = &world->velocities[player];
        ColliderComponent *player_collider = &world->colliders[player];
        HealthComponent *player_health = &world->health[player];
        RenderComponent *player_render = &world->renders[player];

        float input_x = 0.0f;
        if (IsKeyDown(KEY_A)) {
            input_x -= 1.0f;
        }
        if (IsKeyDown(KEY_D)) {
            input_x += 1.0f;
        }

        if (input_x != 0.0f) {
            player_velocity->value.x += input_x * MOVE_ACCELERATION * delta_time;
            player_velocity->value.x = clamp_float(player_velocity->value.x, -MAX_RUN_SPEED, MAX_RUN_SPEED);
        } else {
            const float friction = player_collider->is_grounded ? GROUND_FRICTION : AIR_FRICTION;
            player_velocity->value.x = move_toward_zero(player_velocity->value.x, friction * delta_time);
        }

        if (IsKeyPressed(KEY_W) && player_collider->is_grounded) {
            player_velocity->value.y = JUMP_VELOCITY;
            player_collider->is_grounded = false;
        }

        if (IsKeyPressed(KEY_K)) {
            player_health->current = clamp_int(player_health->current - 10, 0, player_health->max);
        }
        if (IsKeyPressed(KEY_L)) {
            player_health->current = clamp_int(player_health->current + 10, 0, player_health->max);
        }

        player_velocity->value.y += GRAVITY * delta_time;

        if (IsKeyDown(KEY_S) && !player_collider->is_grounded) {
            player_velocity->value.y += FAST_FALL_ACCELERATION * delta_time;
        }

        player_position->value.x += player_velocity->value.x * delta_time;
        if (player_velocity->value.x > 0.0f) {
            const int right_tile = world_to_tile(player_position->value.x + player_collider->size.x - 0.001f);
            if (player_hits_horizontal_tile(player_position->value, player_collider->size, right_tile)) {
                player_position->value.x = right_tile * TILE_SIZE - player_collider->size.x;
                player_velocity->value.x = 0.0f;
            }
        } else if (player_velocity->value.x < 0.0f) {
            const int left_tile = world_to_tile(player_position->value.x);
            if (player_hits_horizontal_tile(player_position->value, player_collider->size, left_tile)) {
                player_position->value.x = (left_tile + 1) * TILE_SIZE;
                player_velocity->value.x = 0.0f;
            }
        }

        player_position->value.y += player_velocity->value.y * delta_time;
        player_collider->is_grounded = false;
        if (player_velocity->value.y > 0.0f) {
            const int bottom_tile = world_to_tile(player_position->value.y + player_collider->size.y - 0.001f);
            if (player_hits_vertical_tile(player_position->value, player_collider->size, bottom_tile)) {
                player_position->value.y = bottom_tile * TILE_SIZE - player_collider->size.y;
                player_velocity->value.y = 0.0f;
                player_collider->is_grounded = true;
            }
        } else if (player_velocity->value.y < 0.0f) {
            const int top_tile = world_to_tile(player_position->value.y);
            if (player_hits_vertical_tile(player_position->value, player_collider->size, top_tile)) {
                player_position->value.y = (top_tile + 1) * TILE_SIZE;
                player_velocity->value.y = 0.0f;
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
        DrawRectangleV(player_position->value, player_render->size, player_render->color);

        const int health_bar_x = 20;
        const int health_bar_y = 20;
        const int health_bar_width = 220;
        const int health_bar_height = 24;
        const float health_percent = (float)player_health->current / (float)player_health->max;
        const int current_health_width = (int)(health_bar_width * health_percent);

        DrawRectangle(health_bar_x, health_bar_y, health_bar_width, health_bar_height, DARKGRAY);
        DrawRectangle(health_bar_x, health_bar_y, current_health_width, health_bar_height, RED);
        DrawRectangleLines(health_bar_x, health_bar_y, health_bar_width, health_bar_height, BLACK);
        DrawText(TextFormat("HP %d/%d", player_health->current, player_health->max),
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
