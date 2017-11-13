#include <libtcod.h>
#include <stdio.h>

#include "CMemLeak.h"
#include "game.h"

void ai_monster(actor_t *actor)
{
    bool chasing = false;

    for (void **i = TCOD_list_begin(actor->map->actors); i != TCOD_list_end(actor->map->actors); i++)
    {
        actor_t *other = *i;

        // TODO: determine if other is hostile somehow
        if (other != actor && (other == player || other->ai == ai_pet))
        {
            if (TCOD_map_is_in_fov(actor->fov_map, other->x, other->y))
            {
                msg_log(actor->map, actor->x, actor->y, TCOD_white, "%s engages %s", actor->name, other->name);

                interactions_t interactions = INTERACTIONS_NONE;

                interactions.attack = true;

                actor_target_set(actor, other->x, other->y, interactions);

                chasing = true;

                break;
            }
        }
    }

    if (!chasing)
    {
        switch (TCOD_random_get_int(NULL, 0, 8))
        {
        case 0:
            actor_target_set(actor, actor->x, actor->y - 1, INTERACTIONS_NONE);

            break;

        case 1:
            actor_target_set(actor, actor->x, actor->y + 1, INTERACTIONS_NONE);

            break;

        case 2:
            actor_target_set(actor, actor->x - 1, actor->y, INTERACTIONS_NONE);

            break;

        case 3:
            actor_target_set(actor, actor->x + 1, actor->y, INTERACTIONS_NONE);

            break;
        }
    }

    actor_target_process(actor);
}

void ai_pet(actor_t *actor)
{
    bool chasing = false;

    for (void **i = TCOD_list_begin(actor->map->actors); i != TCOD_list_end(actor->map->actors); i++)
    {
        actor_t *other = *i;

        if (other != actor && other->ai == ai_monster)
        {
            if (TCOD_map_is_in_fov(actor->fov_map, other->x, other->y))
            {
                msg_log(actor->map, actor->x, actor->y, TCOD_white, "%s engages %s", actor->name, other->name);

                interactions_t interactions = INTERACTIONS_NONE;

                interactions.attack = true;

                actor_target_set(actor, other->x, other->y, interactions);

                chasing = true;

                break;
            }
        }
    }

    if (!chasing)
    {
        actor_target_set(actor, player->x, player->y, INTERACTIONS_NONE);
    }

    actor_target_process(actor);
}