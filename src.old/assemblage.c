#include <libtcod/libtcod.h>

#include "assemblage.h"
#include "component.h"
#include "entity.h"
#include "game.h"
#include "map.h"

entity_t *create_player(map_t *map, int x, int y)
{
    entity_t *entity = entity_create(map->game);

    alignment_t *alignment = (alignment_t *)component_add(entity, COMPONENT_ALIGNMENT);
    alignment->type = ALIGNMENT_GOOD;

    appearance_t *appearance = (appearance_t *)component_add(entity, COMPONENT_APPEARANCE);
    appearance->name = "Blinky";
    appearance->glyph = '@';
    appearance->color = TCOD_white;
    appearance->layer = LAYER_1;

    component_add(entity, COMPONENT_CASTER);

    fov_t *fov = (fov_t *)component_add(entity, COMPONENT_FOV);
    fov->radius = 1;

    health_t *health = (health_t *)component_add(entity, COMPONENT_HEALTH);
    health->max = 20;
    health->current = health->max;

    component_add(entity, COMPONENT_INVENTORY);

    light_t *light = (light_t *)component_add(entity, COMPONENT_LIGHT);
    light->radius = 5;
    light->color = TCOD_white;
    light->flicker = false;
    light->priority = LIGHT_PRIORITY_0;

    position_t *position = (position_t *)component_add(entity, COMPONENT_POSITION);
    position->level = map->level;
    position->x = x;
    position->y = y;
    entity_map_place(entity);

    component_add(entity, COMPONENT_SOLID);

    return entity;
}

entity_t *create_pet(map_t *map, int x, int y)
{
    entity_t *entity = entity_create(map->game);

    ai_t *ai = (ai_t *)component_add(entity, COMPONENT_AI);
    ai->energy = 1.0f;
    ai->energy_per_turn = 0.5f;
    ai->follow_target = map->game->player;

    alignment_t *alignment = (alignment_t *)component_add(entity, COMPONENT_ALIGNMENT);
    alignment->type = ALIGNMENT_GOOD;

    appearance_t *appearance = (appearance_t *)component_add(entity, COMPONENT_APPEARANCE);
    appearance->name = "Spot";
    appearance->glyph = 'd';
    appearance->color = TCOD_white;
    appearance->layer = LAYER_1;

    fov_t *fov = (fov_t *)component_add(entity, COMPONENT_FOV);
    fov->radius = 1;

    health_t *health = (health_t *)component_add(entity, COMPONENT_HEALTH);
    health->max = 20;
    health->current = health->max;

    light_t *light = (light_t *)component_add(entity, COMPONENT_LIGHT);
    light->radius = 5;
    light->color = TCOD_white;
    light->flicker = false;
    light->priority = LIGHT_PRIORITY_0;

    position_t *position = (position_t *)component_add(entity, COMPONENT_POSITION);
    position->level = map->level;
    position->x = x;
    position->y = y;
    entity_map_place(entity);

    component_add(entity, COMPONENT_SOLID);

    return entity;
}

entity_t *create_skeleton(map_t *map, int x, int y)
{
    entity_t *entity = entity_create(map->game);

    ai_t *ai = (ai_t *)component_add(entity, COMPONENT_AI);
    ai->energy = 1.0f;
    ai->energy_per_turn = 0.5f;
    ai->follow_target = NULL;

    alignment_t *alignment = (alignment_t *)component_add(entity, COMPONENT_ALIGNMENT);
    alignment->type = ALIGNMENT_EVIL;

    appearance_t *appearance = (appearance_t *)component_add(entity, COMPONENT_APPEARANCE);
    appearance->name = "Skeleton";
    appearance->glyph = 's';
    appearance->color = TCOD_white;
    appearance->layer = LAYER_1;

    fov_t *fov = (fov_t *)component_add(entity, COMPONENT_FOV);
    fov->radius = 5;
    fov->fov_map = NULL;

    health_t *health = (health_t *)component_add(entity, COMPONENT_HEALTH);
    health->max = TCOD_random_get_int(NULL, 10, 20);
    health->current = health->max;

    position_t *position = (position_t *)component_add(entity, COMPONENT_POSITION);
    position->level = map->level;
    position->x = x;
    position->y = y;
    entity_map_place(entity);

    component_add(entity, COMPONENT_SOLID);

    return entity;
}

entity_t *create_skeleton_captain(map_t *map, int x, int y)
{
    entity_t *entity = entity_create(map->game);

    ai_t *ai = (ai_t *)component_add(entity, COMPONENT_AI);
    ai->energy = 1.0f;
    ai->energy_per_turn = 0.75f;
    ai->follow_target = NULL;

    alignment_t *alignment = (alignment_t *)component_add(entity, COMPONENT_ALIGNMENT);
    alignment->type = ALIGNMENT_EVIL;

    appearance_t *appearance = (appearance_t *)component_add(entity, COMPONENT_APPEARANCE);
    appearance->name = "Skeleton Captain";
    appearance->glyph = 'S';
    appearance->color = TCOD_white;
    appearance->layer = LAYER_1;

    fov_t *fov = (fov_t *)component_add(entity, COMPONENT_FOV);
    fov->radius = 5;
    fov->fov_map = NULL;

    health_t *health = (health_t *)component_add(entity, COMPONENT_HEALTH);
    health->max = TCOD_random_get_int(NULL, 15, 25);
    health->current = health->max;

    position_t *position = (position_t *)component_add(entity, COMPONENT_POSITION);
    position->level = map->level;
    position->x = x;
    position->y = y;
    entity_map_place(entity);

    component_add(entity, COMPONENT_SOLID);

    return entity;
}

entity_t *create_zombie(map_t *map, int x, int y)
{
    entity_t *entity = entity_create(map->game);

    ai_t *ai = (ai_t *)component_add(entity, COMPONENT_AI);
    ai->energy = 1.0f;
    ai->energy_per_turn = 0.25f;
    ai->follow_target = NULL;

    alignment_t *alignment = (alignment_t *)component_add(entity, COMPONENT_ALIGNMENT);
    alignment->type = ALIGNMENT_EVIL;

    appearance_t *appearance = (appearance_t *)component_add(entity, COMPONENT_APPEARANCE);
    appearance->name = "Zombie";
    appearance->glyph = 'z';
    appearance->color = TCOD_dark_green;
    appearance->layer = LAYER_1;

    fov_t *fov = (fov_t *)component_add(entity, COMPONENT_FOV);
    fov->radius = 5;
    fov->fov_map = NULL;

    health_t *health = (health_t *)component_add(entity, COMPONENT_HEALTH);
    health->max = TCOD_random_get_int(NULL, 20, 30);
    health->current = health->max;

    position_t *position = (position_t *)component_add(entity, COMPONENT_POSITION);
    position->level = map->level;
    position->x = x;
    position->y = y;
    entity_map_place(entity);

    component_add(entity, COMPONENT_SOLID);

    return entity;
}

entity_t *create_jackal(map_t *map, int x, int y)
{
    entity_t *entity = entity_create(map->game);

    ai_t *ai = (ai_t *)component_add(entity, COMPONENT_AI);
    ai->energy = 1.0f;
    ai->energy_per_turn = 1.5f;
    ai->follow_target = NULL;

    alignment_t *alignment = (alignment_t *)component_add(entity, COMPONENT_ALIGNMENT);
    alignment->type = ALIGNMENT_EVIL;

    appearance_t *appearance = (appearance_t *)component_add(entity, COMPONENT_APPEARANCE);
    appearance->name = "Jackal";
    appearance->glyph = 'j';
    appearance->color = TCOD_dark_orange;
    appearance->layer = LAYER_1;

    fov_t *fov = (fov_t *)component_add(entity, COMPONENT_FOV);
    fov->radius = 5;
    fov->fov_map = NULL;

    health_t *health = (health_t *)component_add(entity, COMPONENT_HEALTH);
    health->max = TCOD_random_get_int(NULL, 5, 10);
    health->current = health->max;

    position_t *position = (position_t *)component_add(entity, COMPONENT_POSITION);
    position->level = map->level;
    position->x = x;
    position->y = y;
    entity_map_place(entity);

    component_add(entity, COMPONENT_SOLID);

    return entity;
}

entity_t *create_adventurer(map_t *map, int x, int y)
{
    entity_t *entity = entity_create(map->game);

    ai_t *ai = (ai_t *)component_add(entity, COMPONENT_AI);
    ai->energy = 1.0f;
    ai->energy_per_turn = 0.5f;
    ai->follow_target = NULL;

    alignment_t *alignment = (alignment_t *)component_add(entity, COMPONENT_ALIGNMENT);
    alignment->type = ALIGNMENT_GOOD;

    appearance_t *appearance = (appearance_t *)component_add(entity, COMPONENT_APPEARANCE);
    appearance->name = "Adventurer";
    appearance->glyph = 'a';
    appearance->color = TCOD_azure;
    appearance->layer = LAYER_1;

    fov_t *fov = (fov_t *)component_add(entity, COMPONENT_FOV);
    fov->radius = 5;
    fov->fov_map = NULL;

    health_t *health = (health_t *)component_add(entity, COMPONENT_HEALTH);
    health->max = TCOD_random_get_int(NULL, 10, 20);
    health->current = health->max;

    position_t *position = (position_t *)component_add(entity, COMPONENT_POSITION);
    position->level = map->level;
    position->x = x;
    position->y = y;
    entity_map_place(entity);

    component_add(entity, COMPONENT_SOLID);

    return entity;
}

entity_t *create_longsword(map_t *map, int x, int y)
{
    entity_t *entity = entity_create(map->game);

    appearance_t *appearance = (appearance_t *)component_add(entity, COMPONENT_APPEARANCE);
    appearance->name = "Longsword";
    appearance->glyph = '|';
    appearance->color = TCOD_white;

    pickable_t *pickable = (pickable_t *)component_add(entity, COMPONENT_PICKABLE);
    pickable->weight = 10.0f;

    position_t *position = (position_t *)component_add(entity, COMPONENT_POSITION);
    position->level = map->level;
    position->x = x;
    position->y = y;
    entity_map_place(entity);

    return entity;
}

entity_t *create_brazier(map_t *map, int x, int y)
{
    entity_t *entity = entity_create(map->game);

    appearance_t *appearance = (appearance_t *)component_add(entity, COMPONENT_APPEARANCE);
    appearance->name = "Brazier";
    appearance->glyph = '*';
    appearance->color = TCOD_color_RGB(
        (uint8)TCOD_random_get_int(NULL, 0, 255),
        (uint8)TCOD_random_get_int(NULL, 0, 255),
        (uint8)TCOD_random_get_int(NULL, 0, 255));

    light_t *light = (light_t *)component_add(entity, COMPONENT_LIGHT);
    light->radius = TCOD_random_get_int(NULL, 10, 20);
    light->color = appearance->color;
    light->flicker = false;
    light->priority = LIGHT_PRIORITY_1;
    light->fov_map = NULL;

    component_add(entity, COMPONENT_OPAQUE);

    position_t *position = (position_t *)component_add(entity, COMPONENT_POSITION);
    position->level = map->level;
    position->x = x;
    position->y = y;
    entity_map_place(entity);

    component_add(entity, COMPONENT_SOLID);

    return entity;
}
