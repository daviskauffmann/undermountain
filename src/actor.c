#include <libtcod/libtcod.h>
#include <malloc.h>

#include "actor.h"
#include "game.h"

struct actor *actor_create(enum actor_type type, struct game *game, int level, int x, int y, int health, enum faction faction)
{
    struct actor *actor = malloc(sizeof(struct actor));

    actor->type = type;
    actor->game = game;
    actor->level = level;
    actor->x = x;
    actor->y = y;
    actor->health = health;
    actor->faction = faction;
    actor->torch_fov = NULL;
    actor->items = TCOD_list_new();

    return actor;
}

void actor_destroy(struct actor *actor)
{
    if (actor->torch_fov != NULL)
    {
        TCOD_map_delete(actor->torch_fov);
    }

    TCOD_list_delete(actor->items);

    free(actor);
}
