#include <libtcod.h>

#include "actor.h"
#include "config.h"
#include "game.h"
#include "map.h"

actor_t *actor_create(map_t *map, actortype_t type, int x, int y, unsigned char glyph, TCOD_color_t color, int sight_radius)
{
    actor_t *actor = (actor_t *)malloc(sizeof(actor_t));

    actor->type = type;
    actor->x = x;
    actor->y = y;

    TCOD_list_push(map->actors, actor);

    return actor;
}

void actor_destroy(map_t *map, actor_t *actor)
{
    TCOD_list_remove(map->actors, actor);

    free(actor);
}

void actor_move(map_t *map, actor_t *actor, int x, int y)
{
    if (x < 0 || x >= MAP_WIDTH || y < 0 || y >= MAP_HEIGHT)
    {
        return;
    }

    tile_t *tile = &map->tiles[x][y];
    if (!tileinfo[tile->type].is_walkable)
    {
        return;
    }

    for (actor_t **iterator = (actor_t **)TCOD_list_begin(map->actors);
         iterator != (actor_t **)TCOD_list_end(map->actors);
         iterator++)
    {
        actor_t *other = *iterator;

        if (other->x != x || other->y != y)
        {
            continue;
        }

        // TODO: damage and health
        // TODO: player death
        // TODO: dealing with corpses, is_dead flag or separate object altogether?
        // if corpses can be resurrected, they will need to store information about the actor
        // if corpses can be picked up, they will need to act like items
        if (other != player)
        {
            actor_destroy(map, other);
        }

        return;
    }

    actor->x = x;
    actor->y = y;
}