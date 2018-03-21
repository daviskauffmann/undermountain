#include <libtcod/libtcod.h>
#include <malloc.h>

#include "actor.h"
#include "map.h"

struct actor *actor_create(enum actor_type type, struct map *map, int x, int y, int health, enum faction faction)
{
    struct actor *actor = malloc(sizeof(struct actor));

    actor->type = type;
    actor->map = map;
    actor->x = x;
    actor->y = y;
    actor->health = health;
    actor->faction = faction;
    actor->torch_fov = NULL;
    actor->items = TCOD_list_new();

    TCOD_list_push(actor->map->actors, actor);
    TCOD_list_push(actor->map->tiles[actor->x + actor->y * MAP_WIDTH].actors, actor);

    return actor;
}

void actor_destroy(struct actor *actor)
{
    TCOD_list_remove(actor->map->actors, actor);
    TCOD_list_remove(actor->map->tiles[actor->x + actor->y * MAP_WIDTH].actors, actor);

    if (actor->torch_fov != NULL)
    {
        TCOD_map_delete(actor->torch_fov);
    }

    TCOD_list_delete(actor->items);

    free(actor);
}
