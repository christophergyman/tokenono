#include "ecs.h"

#include <stdio.h>
#include <stdlib.h>

static void ecs_abort(const char *message)
{
    fprintf(stderr, "ecs error: %s\n", message);
    abort();
}

EcsWorld *ecs_world_create(Arena *arena, unsigned int capacity)
{
    if (capacity == 0) {
        ecs_abort("world capacity must be greater than zero");
    }

    EcsWorld *world = arena_alloc(arena, sizeof(*world));
    world->capacity = capacity;
    world->alive = arena_alloc(arena, sizeof(*world->alive) * capacity);
    world->masks = arena_alloc(arena, sizeof(*world->masks) * capacity);
    world->positions = arena_alloc(arena, sizeof(*world->positions) * capacity);
    world->velocities = arena_alloc(arena, sizeof(*world->velocities) * capacity);
    world->colliders = arena_alloc(arena, sizeof(*world->colliders) * capacity);
    world->health = arena_alloc(arena, sizeof(*world->health) * capacity);
    world->renders = arena_alloc(arena, sizeof(*world->renders) * capacity);
    world->damage = arena_alloc(arena, sizeof(*world->damage) * capacity);

    return world;
}

Entity ecs_create_entity(EcsWorld *world)
{
    if (world == NULL) {
        ecs_abort("world is NULL during entity creation");
    }

    for (Entity entity = 0; entity < world->capacity; entity++) {
        if (!world->alive[entity]) {
            world->alive[entity] = true;
            world->masks[entity] = 0;
            return entity;
        }
    }

    ecs_abort("entity capacity exhausted");
    return INVALID_ENTITY;
}

void ecs_destroy_entity(EcsWorld *world, Entity entity)
{
    if (!ecs_entity_is_valid(world, entity)) {
        ecs_abort("cannot destroy invalid entity");
    }

    world->alive[entity] = false;
    world->masks[entity] = 0;
}

bool ecs_entity_is_valid(const EcsWorld *world, Entity entity)
{
    return world != NULL && entity < world->capacity && world->alive[entity];
}

void ecs_add_components(EcsWorld *world, Entity entity, ComponentMask components)
{
    if (!ecs_entity_is_valid(world, entity)) {
        ecs_abort("cannot add components to invalid entity");
    }

    world->masks[entity] |= components;
}

bool ecs_has_components(const EcsWorld *world, Entity entity, ComponentMask components)
{
    if (!ecs_entity_is_valid(world, entity)) {
        return false;
    }

    return (world->masks[entity] & components) == components;
}
