#ifndef ECS_H
#define ECS_H

#include <raylib.h>

#include <stdbool.h>
#include <limits.h>

#include "arena.h"

#define ECS_MAX_ENTITIES 512u
#define INVALID_ENTITY UINT_MAX

typedef unsigned int Entity;
typedef unsigned int ComponentMask;

enum {
    COMPONENT_POSITION = 1u << 0,
    COMPONENT_VELOCITY = 1u << 1,
    COMPONENT_COLLIDER = 1u << 2,
    COMPONENT_HEALTH = 1u << 3,
    COMPONENT_PLAYER = 1u << 4,
    COMPONENT_ENEMY = 1u << 5,
    COMPONENT_RENDER = 1u << 6,
    COMPONENT_DAMAGE = 1u << 7,
};

typedef struct PositionComponent {
    Vector2 value;
} PositionComponent;

typedef struct VelocityComponent {
    Vector2 value;
} VelocityComponent;

typedef struct ColliderComponent {
    Vector2 size;
    bool is_grounded;
} ColliderComponent;

typedef struct HealthComponent {
    int current;
    int max;
} HealthComponent;

typedef struct RenderComponent {
    Vector2 size;
    Color color;
} RenderComponent;

typedef struct DamageComponent {
    int amount;
} DamageComponent;

typedef struct EcsWorld {
    unsigned int capacity;
    bool *alive;
    ComponentMask *masks;

    PositionComponent *positions;
    VelocityComponent *velocities;
    ColliderComponent *colliders;
    HealthComponent *health;
    RenderComponent *renders;
    DamageComponent *damage;
} EcsWorld;

EcsWorld *ecs_world_create(Arena *arena, unsigned int capacity);
Entity ecs_create_entity(EcsWorld *world);
void ecs_destroy_entity(EcsWorld *world, Entity entity);
bool ecs_entity_is_valid(const EcsWorld *world, Entity entity);
void ecs_add_components(EcsWorld *world, Entity entity, ComponentMask components);
bool ecs_has_components(const EcsWorld *world, Entity entity, ComponentMask components);

#endif
