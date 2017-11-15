#include <math.h>
#include <libtcod.h>

#include "CMemLeak.h"
#include "game.h"
#include "utils.h"

void ai_monster(actor_t *actor)
{
    for (void **i = TCOD_list_begin(actor->map->actors); i != TCOD_list_end(actor->map->actors); i++)
    {
        actor_t *other = *i;

        if (other != actor && TCOD_map_is_in_fov(actor->fov_map, other->x, other->y) && other == player)
        {
            if (distance(actor->x, actor->y, other->x, other->y) < 2.0f)
            {
                actor_attack(actor, other);
            }
            else
            {
                actor_path_towards(actor, other->x, other->y);
            }
        }
    }
}

void ai_pet(actor_t *actor)
{
    bool enemy_found = false;

    for (void **i = TCOD_list_begin(actor->map->actors); i != TCOD_list_end(actor->map->actors); i++)
    {
        actor_t *other = *i;

        if (other != actor && TCOD_map_is_in_fov(actor->fov_map, other->x, other->y) && other != player)
        {
            enemy_found = true;

            if (distance(actor->x, actor->y, other->x, other->y) < 2.0f)
            {
                actor_attack(actor, other);
            }
            else
            {
                actor_path_towards(actor, other->x, other->y);
            }
        }
    }

    if (!enemy_found)
    {
        actor_path_towards(actor, player->x, player->y);
    }
}