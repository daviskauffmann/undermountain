#include <stdint.h>
#include <libtcod.h>

#include "config.h"
#include "world.h"

actor_t *actor_create(map_t *map, uint8_t x, uint8_t y, uint8_t glyph, TCOD_color_t color, uint8_t sight_radius)
{
    actor_t *actor = (actor_t *)malloc(sizeof(actor_t));

    actor->x = x;
    actor->y = y;
    actor->glyph = glyph;
    actor->color = color;
    actor->sight_radius = sight_radius;

    TCOD_list_push(map->actors, actor);

    return actor;
}

void actor_destroy(map_t *map, actor_t *actor)
{
    TCOD_list_remove(map->actors, actor);

    free(actor);
}

void actor_move(map_t *map, actor_t *actor, uint8_t x, uint8_t y)
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
        // TODO: dealing with corpses, is_dead flag or separate object alltogether?
        // if corpses can be resurrected, they will need to store information about the actor
        // if corpses can be picked up, they will need to act like items
        actor_destroy(map, other);

        return;
    }

    actor->x = x;
    actor->y = y;
}