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
                msg_log("{name} spots {name}", actor->map, actor->x, actor->y);

                interactions_t interactions = {
                    .descend = false,
                    .ascend = false,
                    .light_on = false,
                    .light_off = false,
                    .attack = true,
                    .take_item = false,
                    .take_items = false};

                actor_target_set(actor, other->x, other->y, interactions);

                chasing = true;

                break;
            }
        }
    }

    if (!chasing)
    {
        interactions_t interactions = {
            .descend = false,
            .ascend = false,
            .light_on = false,
            .light_off = false,
            .attack = false,
            .take_item = false,
            .take_items = false};

        switch (TCOD_random_get_int(NULL, 0, 8))
        {
        case 0:
            actor_target_set(actor, actor->x, actor->y - 1, interactions);

            break;

        case 1:
            actor_target_set(actor, actor->x, actor->y + 1, interactions);

            break;

        case 2:
            actor_target_set(actor, actor->x - 1, actor->y, interactions);

            break;

        case 3:
            actor_target_set(actor, actor->x + 1, actor->y, interactions);

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
                msg_log("{pet} spots {name}", actor->map, actor->x, actor->y);

                interactions_t interactions = {
                    .descend = false,
                    .ascend = false,
                    .light_on = false,
                    .light_off = false,
                    .attack = true,
                    .take_item = false,
                    .take_items = false};

                actor_target_set(actor, other->x, other->y, interactions);

                chasing = true;

                break;
            }
        }
    }

    if (!chasing)
    {
        interactions_t interactions = {
            .descend = false,
            .ascend = false,
            .light_on = false,
            .light_off = false,
            .attack = false,
            .take_item = false,
            .take_items = false};

        actor_target_set(actor, player->x, player->y, interactions);
    }

    actor_target_process(actor);
}