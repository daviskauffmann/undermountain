#include <stdio.h>
#include <stdint.h>
#include <libtcod.h>

#include "config.h"
#include "game.h"
#include "world.h"

world_t *world_create(void)
{
    world_t *world = (world_t *)malloc(sizeof(world_t));

    world->maps = TCOD_list_new();

    return world;
}

void world_update(void)
{
    for (map_t **iterator = (map_t **)TCOD_list_begin(world->maps);
         iterator != (map_t **)TCOD_list_end(world->maps);
         iterator++)
    {
        map_t *map = *iterator;

        for (actor_t **iterator = (actor_t **)TCOD_list_begin(map->actors);
             iterator != (actor_t **)TCOD_list_end(map->actors);
             iterator++)
        {
            actor_t *actor = *iterator;

            if (actor == player)
            {
                continue;
            }

            if (TCOD_random_get_int(NULL, 0, 1) == 0)
            {
                TCOD_map_t TCOD_map = map_to_TCOD_map(map);

                map_calc_fov(TCOD_map, actor->x, actor->y, actor->sight_radius);

                for (actor_t **iterator = (actor_t **)TCOD_list_begin(map->actors);
                     iterator != (actor_t **)TCOD_list_end(map->actors);
                     iterator++)
                {
                    actor_t *other = *iterator;

                    if (other == actor)
                    {
                        continue;
                    }

                    if (TCOD_map_is_in_fov(TCOD_map, other->x, other->y))
                    {
                        // TODO: maybe store the path on the actor somehow so it can be reused
                        TCOD_path_t path = map_calc_path(TCOD_map, actor->x, actor->y, other->x, other->y);

                        if (TCOD_path_is_empty(path))
                        {
                            goto end;
                        }

                        int x, y;
                        if (!TCOD_path_walk(path, &x, &y, false))
                        {
                            goto end;
                        }

                        actor_move(map, actor, x, y);

                    end:
                        TCOD_path_delete(path);

                        break;
                    }
                }

                TCOD_map_delete(TCOD_map);
            }
            else
            {
                int dir = TCOD_random_get_int(NULL, 0, 8);
                switch (dir)
                {
                case 0:
                    actor_move(map, actor, actor->x, actor->y - 1);
                    break;
                case 1:
                    actor_move(map, actor, actor->x, actor->y + 1);
                    break;
                case 2:
                    actor_move(map, actor, actor->x - 1, actor->y);
                    break;
                case 3:
                    actor_move(map, actor, actor->x + 1, actor->y);
                    break;
                }
            }
        }
    }
}

void world_destroy(void)
{
    for (map_t **iterator = (map_t **)TCOD_list_begin(world->maps);
         iterator != (map_t **)TCOD_list_end(world->maps);
         iterator++)
    {
        map_t *map = *iterator;

        TCOD_list_clear_and_delete(map->actors);
    }

    TCOD_list_clear_and_delete(world->maps);

    free(world);
}