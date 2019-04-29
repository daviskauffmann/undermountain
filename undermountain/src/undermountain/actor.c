#include <undermountain/undermountain.h>

// TODO: actors should ascend/descend with their leader

static int calc_ability_modifier(int ability);

struct actor *actor_create(const char *name, enum race race, enum class class, enum faction faction, int level, int floor, int x, int y)
{
    struct actor *actor = malloc(sizeof(struct actor));

    if (!actor)
    {
        printf("Couldn't allocate actor\n");

        return NULL;
    }

    actor->name = _strdup(name);
    actor->race = race;
    actor->class = class;
    actor->faction = faction;
    actor->level = level;
    actor->experience = (actor->level - 1) * 1000;
    for (enum ability ability = 0; ability < NUM_ABILITIES; ability++)
    {
        actor->ability_scores[ability] = 10;
    }
    actor->base_hp = class_info[actor->class].hit_die * actor->level;
    actor->current_hp = actor_calc_max_hp(actor);
    for (enum equip_slot equip_slot = 0; equip_slot < NUM_EQUIP_SLOTS; equip_slot++)
    {
        actor->equipment[equip_slot] = NULL;
    }
    actor->items = TCOD_list_new();
    actor->floor = floor;
    actor->x = x;
    actor->y = y;
    actor->speed = TCOD_random_get_float(NULL, 0.2f, 0.8f);
    actor->energy = 1.0f;
    actor->last_seen_x = -1;
    actor->last_seen_y = -1;
    actor->turns_chased = 0;
    actor->leader = NULL;
    actor->kills = 0;
    actor->glow = false;
    actor->glow_fov = NULL;
    actor->torch = TCOD_random_get_int(NULL, 0, 20) == 0;
    actor->torch_fov = NULL;
    actor->fov = NULL;
    actor->flash_fade = 0;
    actor->dead = false;

    actor_calc_light(actor);
    actor_calc_fov(actor);

    return actor;
}

void actor_level_up(struct actor *actor)
{
    actor->level++;
    actor->base_hp += roll(1, class_info[actor->class].hit_die);
    actor->current_hp = actor_calc_max_hp(actor);

    game_log(
        actor->floor,
        actor->x,
        actor->y,
        TCOD_yellow,
        "%s has gained a level!",
        actor->name);
}

int actor_calc_max_hp(struct actor *actor)
{
    return actor->base_hp + calc_ability_modifier(actor->ability_scores[ABILITY_CONSTITUTION]);
}

int actor_calc_enhancement_bonus(struct actor *actor)
{
    int bonus = 0;

    for (enum equip_slot equip_slot = 0; equip_slot < NUM_EQUIP_SLOTS; equip_slot++)
    {
        struct item *equipment = actor->equipment[equip_slot];

        if (equipment)
        {
            TCOD_list_t item_properties = item_info[equipment->type].item_properties;

            TCOD_LIST_FOREACH(item_properties)
            {
                struct base_item_property *base_item_property = *iterator;

                if (base_item_property->item_property == ITEM_PROPERTY_ENHANCEMENT_BONUS)
                {
                    struct enhancement_bonus *enhancement_bonus = (struct enhancement_bonus *)base_item_property;

                    bonus += enhancement_bonus->bonus;
                }
            }
        }
    }

    return bonus;
}

int actor_calc_attack_bonus(struct actor *actor)
{
    // TODO: base attack bonus based on class
    int base_attack_bonus = 0;

    return base_attack_bonus + calc_ability_modifier(actor->ability_scores[ABILITY_STRENGTH]) + actor_calc_enhancement_bonus(actor);
}

int actor_calc_armor_class(struct actor *actor)
{
    int ac = 10;

    for (int i = 0; i < NUM_EQUIP_SLOTS; i++)
    {
        struct item *equipment = actor->equipment[i];

        if (equipment)
        {
            enum base_item base_item = item_info[equipment->type].base_item;

            ac += base_item_info[base_item].base_ac;

            TCOD_list_t item_properties = item_info[equipment->type].item_properties;

            TCOD_LIST_FOREACH(item_properties)
            {
                struct base_item_property *base_item_property = *iterator;

                if (base_item_property->item_property == ITEM_PROPERTY_AC_BONUS)
                {
                    struct ac_bonus *ac_bonus = (struct ac_bonus *)base_item_property;

                    // TODO: deal with stacking AC types
                    ac += ac_bonus->bonus;
                }
            }
        }
    }

    return ac;
}

void actor_calc_weapon(struct actor *actor, int *num_dice, int *die_to_roll, int *crit_threat, int *crit_mult, bool ranged)
{
    struct item *weapon = actor->equipment[EQUIP_SLOT_MAIN_HAND];

    if (ranged)
    {
        *num_dice = 0;
        *die_to_roll = 0;
        *crit_threat = 0;
        *crit_mult = 0;
    }
    else
    {
        *num_dice = 1;
        *die_to_roll = 3;
        *crit_threat = 20;
        *crit_mult = 2;
    }

    if (weapon)
    {
        enum base_item base_item = item_info[weapon->type].base_item;

        if (base_item_info[base_item].ranged == ranged)
        {
            *num_dice = base_item_info[base_item].num_dice;
            *die_to_roll = base_item_info[base_item].die_to_roll;
            *crit_threat = base_item_info[base_item].crit_threat;
            *crit_mult = base_item_info[base_item].crit_mult;
        }
    }
}

int actor_calc_damage_bonus(struct actor *actor)
{
    return calc_ability_modifier(actor->ability_scores[ABILITY_STRENGTH]) + actor_calc_enhancement_bonus(actor);
}

void actor_update_flash(struct actor *actor)
{
    if (actor->flash_fade > 0)
    {
        actor->flash_fade -= 4.0f * TCOD_sys_get_last_frame_length();

        // game->state == GAME_STATE_WAIT;
    }
    else
    {
        actor->flash_fade = 0.0f;
    }
}

void actor_calc_light(struct actor *actor)
{
    struct map *map = &game->maps[actor->floor];

    if (actor->glow_fov)
    {
        TCOD_map_delete(actor->glow_fov);

        actor->glow_fov = NULL;
    }

    if (actor->torch_fov)
    {
        TCOD_map_delete(actor->torch_fov);

        actor->torch_fov = NULL;
    }

    if (actor->torch)
    {
        actor->torch_fov = map_to_fov_map(map, actor->x, actor->y, actor_common.torch_radius);
    }
    else if (actor->glow)
    {
        actor->glow_fov = map_to_fov_map(map, actor->x, actor->y, actor_common.glow_radius);
    }
}

void actor_calc_fov(struct actor *actor)
{
    struct map *map = &game->maps[actor->floor];

    if (actor->fov)
    {
        TCOD_map_delete(actor->fov);
    }

    actor->fov = map_to_fov_map(map, actor->x, actor->y, 1);

    TCOD_map_t los_map = map_to_fov_map(map, actor->x, actor->y, 0);

    for (int x = 0; x < MAP_WIDTH; x++)
    {
        for (int y = 0; y < MAP_HEIGHT; y++)
        {
            if (!TCOD_map_is_in_fov(actor->fov, x, y) && TCOD_map_is_in_fov(los_map, x, y))
            {
                TCOD_LIST_FOREACH(map->objects)
                {
                    struct object *object = *iterator;

                    if (object->light_fov && TCOD_map_is_in_fov(object->light_fov, x, y))
                    {
                        TCOD_map_set_in_fov(actor->fov, x, y, true);
                    }
                }

                TCOD_LIST_FOREACH(map->actors)
                {
                    struct actor *other = *iterator;

                    if (other->glow_fov && TCOD_map_is_in_fov(other->glow_fov, x, y))
                    {
                        TCOD_map_set_in_fov(actor->fov, x, y, true);
                    }

                    if (other->torch_fov && TCOD_map_is_in_fov(other->torch_fov, x, y))
                    {
                        TCOD_map_set_in_fov(actor->fov, x, y, true);
                    }
                }
            }
        }
    }

    TCOD_map_delete(los_map);
}

void actor_ai(struct actor *actor)
{
    if (actor == game->player || actor->dead)
    {
        return;
    }

    struct map *map = &game->maps[actor->floor];
    struct tile *tile = &map->tiles[actor->x][actor->y];

    actor->energy += actor->speed;

    while (actor->energy >= 1.0f)
    {
        actor->energy -= 1.0f;

        // look for fountains to heal if low health
        if (actor->current_hp < actor_calc_max_hp(actor) / 2)
        {
            struct object *target = NULL;
            float min_distance = FLT_MAX;

            TCOD_LIST_FOREACH(map->objects)
            {
                struct object *object = *iterator;

                if (TCOD_map_is_in_fov(actor->fov, object->x, object->y) &&
                    object->type == OBJECT_TYPE_FOUNTAIN)
                {
                    float distance = distance_between_sq(actor->x, actor->y, object->x, object->y);

                    if (distance < min_distance)
                    {
                        target = object;
                        min_distance = distance;
                    }
                }
            }

            if (target)
            {
                if (distance_between(actor->x, actor->y, target->x, target->y) < 2.0f)
                {
                    if (actor_drink(actor, target->x, target->y))
                    {
                        continue;
                    }
                }
                else
                {
                    if (actor_path_towards(actor, target->x, target->y))
                    {
                        continue;
                    }
                }
            }
        }

        // look for hostile targets
        {
            struct actor *target = NULL;
            float min_distance = FLT_MAX;

            TCOD_LIST_FOREACH(map->actors)
            {
                struct actor *other = *iterator;

                if (TCOD_map_is_in_fov(actor->fov, other->x, other->y) &&
                    other->faction != actor->faction &&
                    !other->dead)
                {
                    float distance = distance_between_sq(actor->x, actor->y, other->x, other->y);

                    if (distance < min_distance)
                    {
                        target = other;
                        min_distance = distance;
                    }
                }
            }

            if (target)
            {
                actor->last_seen_x = target->x;
                actor->last_seen_y = target->y;
                actor->turns_chased = 0;

                // TODO: if carrying a ranged weapon, actor might want to prioritize retreating rather than perfoming a melee attack
                if (distance_between(actor->x, actor->y, target->x, target->y) < 2.0f &&
                    actor_attack(actor, target, false))
                {
                    continue;
                }

                struct item *weapon = actor->equipment[EQUIP_SLOT_MAIN_HAND];

                if (weapon)
                {
                    enum base_item base_item = item_info[weapon->type].base_item;

                    if (base_item_info[base_item].ranged && actor_shoot(actor, target->x, target->y, NULL, NULL))
                    {
                        continue;
                    }
                }

                if (actor_path_towards(actor, target->x, target->y))
                {
                    continue;
                }
            }
        }

        // go to where a hostile target was recently seen
        if (actor->last_seen_x != -1 && actor->last_seen_y != -1)
        {
            if ((actor->x == actor->last_seen_x && actor->y == actor->last_seen_y) ||
                actor->turns_chased > actor_common.turns_to_chase)
            {
                actor->last_seen_x = -1;
                actor->last_seen_y = -1;
            }
            else if (actor_path_towards(actor, actor->last_seen_x, actor->last_seen_y))
            {
                actor->turns_chased++;

                continue;
            }
        }

        // stay in visiblity/proximity to leader
        if (actor->leader)
        {
            if (!TCOD_map_is_in_fov(actor->fov, actor->leader->x, actor->leader->y) ||
                distance_between(actor->x, actor->y, actor->leader->x, actor->leader->y) > 5.0f)
            {
                if (actor_path_towards(actor, actor->leader->x, actor->leader->y))
                {
                    continue;
                }
            }
        }

        // TODO: move between floors (deferred until processing of inactive maps is figured out)
        // if (tile->object)
        // {
        //     switch (tile->object->type)
        //     {
        //     case OBJECT_TYPE_STAIR_DOWN:
        //     {
        //         if (actor_descend(actor))
        //         {
        //             continue;
        //         }
        //     }
        //     break;
        //     case OBJECT_TYPE_STAIR_UP:
        //     {
        //         if (actor_ascend(actor))
        //         {
        //             continue;
        //         }
        //     }
        //     break;
        //     }
        // }

        // TODO: look for objects to interact with if needed

        // TODO: look for items to pick up

        // pick up items on ground
        if (TCOD_list_size(tile->items) > 0)
        {
            if (actor_grab(actor, actor->x, actor->y))
            {
                continue;
            }
        }

        // move randomly
        if (TCOD_random_get_int(NULL, 0, 1) == 0)
        {
            int x = actor->x + TCOD_random_get_int(NULL, -1, 1);
            int y = actor->y + TCOD_random_get_int(NULL, -1, 1);

            actor_move(actor, x, y);

            continue;
        }
    }
}

bool actor_path_towards(struct actor *actor, int x, int y)
{
    struct map *map = &game->maps[actor->floor];

    TCOD_map_t TCOD_map = map_to_TCOD_map(map);
    TCOD_map_set_properties(TCOD_map, x, y, TCOD_map_is_transparent(TCOD_map, x, y), true);

    TCOD_path_t path = TCOD_path_new_using_map(TCOD_map, 1.0f);
    TCOD_path_compute(path, actor->x, actor->y, x, y);

    bool success = false;

    int next_x, next_y;
    if (!TCOD_path_is_empty(path) && TCOD_path_walk(path, &next_x, &next_y, false))
    {
        success = actor_move(actor, next_x, next_y);
    }
    else
    {
        success = actor_move_towards(actor, x, y);
    }

    TCOD_path_delete(path);

    TCOD_map_delete(TCOD_map);

    return success;
}

bool actor_move_towards(struct actor *actor, int x, int y)
{
    int dx = x - actor->x;
    int dy = y - actor->y;
    float d = distance_between(actor->x, actor->y, x, y);

    if (d > 0.0f)
    {
        dx = (int)round(dx / d);
        dy = (int)round(dy / d);

        return actor_move(actor, actor->x + dx, actor->y + dy);
    }

    return false;
}

bool actor_move(struct actor *actor, int x, int y)
{
    if (!map_is_inside(x, y))
    {
        return false;
    }

    struct map *map = &game->maps[actor->floor];
    struct tile *tile = &map->tiles[x][y];

    if (!tile_info[tile->type].is_walkable)
    {
        return false;
    }

    if (tile->object)
    {
        switch (tile->object->type)
        {
        case OBJECT_TYPE_ALTAR:
        {
            return actor_pray(actor, x, y);
        }
        break;
        case OBJECT_TYPE_CHEST:
        {
            return actor_open_chest(actor, x, y);
        }
        break;
        case OBJECT_TYPE_DOOR_CLOSED:
        {
            return actor_open_door(actor, x, y);
        }
        break;
        case OBJECT_TYPE_FOUNTAIN:
        {
            return actor_drink(actor, x, y);
        }
        break;
        case OBJECT_TYPE_THRONE:
        {
            return actor_sit(actor, x, y);
        }
        break;
        }

        if (!object_info[tile->object->type].is_walkable)
        {
            return false;
        }
    }

    if (tile->actor && tile->actor != actor && !tile->actor->dead)
    {
        if (tile->actor->faction == actor->faction)
        {
            return actor_swap(actor, tile->actor);
        }
        else
        {
            return actor_attack(actor, tile->actor, false);
        }
    }

    struct tile *current_tile = &map->tiles[actor->x][actor->y];

    current_tile->actor = NULL;

    actor->x = x;
    actor->y = y;

    tile->actor = actor;

    for (int i = 0; i < NUM_EQUIP_SLOTS; i++)
    {
        struct item *equipment = actor->equipment[i];

        if (equipment)
        {
            equipment->x = actor->x;
            equipment->y = actor->y;
        }
    }

    TCOD_LIST_FOREACH(actor->items)
    {
        struct item *item = *iterator;

        item->x = actor->x;
        item->y = actor->y;
    }

    return true;
}

bool actor_swap(struct actor *actor, struct actor *other)
{
    struct map *map = &game->maps[actor->floor];

    if (other == game->player)
    {
        return false;
    }

    struct tile *tile = &map->tiles[actor->x][actor->y];
    struct tile *other_tile = &map->tiles[other->x][other->y];

    tile->actor = NULL;
    other_tile->actor = NULL;

    int temp_x = actor->x;
    int temp_y = actor->y;

    actor->x = other->x;
    actor->y = other->y;

    other->x = temp_x;
    other->y = temp_y;

    other_tile->actor = actor;
    tile->actor = other;

    for (int i = 0; i < NUM_EQUIP_SLOTS; i++)
    {
        struct item *equipment = actor->equipment[i];

        if (equipment)
        {
            equipment->x = actor->x;
            equipment->y = actor->y;
        }
    }

    TCOD_LIST_FOREACH(actor->items)
    {
        struct item *item = *iterator;

        item->x = actor->x;
        item->y = actor->y;
    }

    for (int i = 0; i < NUM_EQUIP_SLOTS; i++)
    {
        struct item *equipment = other->equipment[i];

        if (equipment)
        {
            equipment->x = other->x;
            equipment->y = other->y;
        }
    }

    TCOD_LIST_FOREACH(other->items)
    {
        struct item *item = *iterator;

        item->x = other->x;
        item->y = other->y;
    }

    game_log(
        actor->floor,
        actor->x,
        actor->y,
        TCOD_white,
        "%s swaps with %s",
        actor->name,
        other->name);

    return true;
}

bool actor_open_door(struct actor *actor, int x, int y)
{
    if (!map_is_inside(x, y))
    {
        return false;
    }

    struct map *map = &game->maps[actor->floor];
    struct tile *tile = &map->tiles[x][y];

    if (tile->object && tile->object->type == OBJECT_TYPE_DOOR_CLOSED)
    {
        tile->object->type = OBJECT_TYPE_DOOR_OPEN;

        game_log(
            actor->floor,
            actor->x,
            actor->y,
            TCOD_white,
            "%s opens the door",
            actor->name);

        return true;
    }

    game_log(
        actor->floor,
        actor->x,
        actor->y,
        TCOD_white,
        "%s can't open the door",
        actor->name);

    return false;
}

bool actor_close_door(struct actor *actor, int x, int y)
{
    if (!map_is_inside(x, y))
    {
        return false;
    }

    struct map *map = &game->maps[actor->floor];
    struct tile *tile = &map->tiles[x][y];

    if (tile->object && tile->object->type == OBJECT_TYPE_DOOR_OPEN)
    {
        tile->object->type = OBJECT_TYPE_DOOR_CLOSED;

        game_log(
            actor->floor,
            actor->x,
            actor->y,
            TCOD_white,
            "%s closes the door",
            actor->name);

        return true;
    }

    game_log(
        actor->floor,
        actor->x,
        actor->y,
        TCOD_white,
        "%s can't close the door",
        actor->name);

    return false;
}

bool actor_descend(struct actor *actor)
{
    struct map *map = &game->maps[actor->floor];
    struct tile *tile = &map->tiles[actor->x][actor->y];

    if (actor->floor >= NUM_MAPS)
    {
        game_log(
            actor->floor,
            actor->x,
            actor->y,
            TCOD_white,
            "%s has reached the end",
            actor->name);

        return false;
    }

    if (tile->object && tile->object->type == OBJECT_TYPE_STAIR_DOWN)
    {
        struct map *next_map = &game->maps[actor->floor + 1];
        struct tile *next_tile = &next_map->tiles[next_map->stair_up_x][next_map->stair_up_y];

        TCOD_list_remove(map->actors, actor);
        tile->actor = NULL;

        actor->floor++;
        actor->x = next_map->stair_up_x;
        actor->y = next_map->stair_up_y;

        TCOD_list_push(next_map->actors, actor);
        next_tile->actor = actor;

        for (int i = 0; i < NUM_EQUIP_SLOTS; i++)
        {
            struct item *equipment = actor->equipment[i];

            if (equipment)
            {
                equipment->floor = actor->floor;
                equipment->x = actor->x;
                equipment->y = actor->y;

                TCOD_list_remove(map->items, equipment);
                TCOD_list_push(next_map->items, equipment);
            }
        }

        TCOD_LIST_FOREACH(actor->items)
        {
            struct item *item = *iterator;

            item->floor = actor->floor;
            item->x = actor->x;
            item->y = actor->y;

            TCOD_list_remove(map->items, item);
            TCOD_list_push(next_map->items, item);
        }

        game_log(
            actor->floor,
            actor->x,
            actor->y,
            TCOD_white,
            "%s descends",
            actor->name);

        return true;
    }

    game_log(
        actor->floor,
        actor->x,
        actor->y,
        TCOD_white,
        "%s can't descend here",
        actor->name);

    return false;
}

bool actor_ascend(struct actor *actor)
{
    struct map *map = &game->maps[actor->floor];
    struct tile *tile = &map->tiles[actor->x][actor->y];

    if (actor->floor == 0)
    {
        game_log(
            actor->floor,
            actor->x,
            actor->y,
            TCOD_white,
            "%s can't go any higher",
            actor->name);

        return false;
    }

    if (tile->object && tile->object->type == OBJECT_TYPE_STAIR_UP)
    {
        struct map *next_map = &game->maps[actor->floor - 1];
        struct tile *next_tile = &next_map->tiles[next_map->stair_up_x][next_map->stair_up_y];

        TCOD_list_remove(map->actors, actor);
        tile->actor = NULL;

        actor->floor--;
        actor->x = next_map->stair_down_x;
        actor->y = next_map->stair_down_y;

        TCOD_list_push(next_map->actors, actor);
        next_tile->actor = actor;

        for (int i = 0; i < NUM_EQUIP_SLOTS; i++)
        {
            struct item *equipment = actor->equipment[i];

            if (equipment)
            {
                equipment->floor = actor->floor;
                equipment->x = actor->x;
                equipment->y = actor->y;

                TCOD_list_remove(map->items, equipment);
                TCOD_list_push(next_map->items, equipment);
            }
        }

        TCOD_LIST_FOREACH(actor->items)
        {
            struct item *item = *iterator;

            item->floor = actor->floor;
            item->x = actor->x;
            item->y = actor->y;

            TCOD_list_remove(map->items, item);
            TCOD_list_push(next_map->items, item);
        }

        game_log(
            actor->floor,
            actor->x,
            actor->y,
            TCOD_white,
            "%s ascends",
            actor->name);

        return true;
    }

    game_log(
        actor->floor,
        actor->x,
        actor->y,
        TCOD_white,
        "%s can't ascend here",
        actor->name);

    return false;
}

bool actor_open_chest(struct actor *actor, int x, int y)
{
    if (!map_is_inside(x, y))
    {
        return false;
    }

    struct map *map = &game->maps[actor->floor];
    struct tile *tile = &map->tiles[x][y];

    if (tile->object && tile->object->type == OBJECT_TYPE_CHEST)
    {
        game_log(
            actor->floor,
            actor->x,
            actor->y,
            TCOD_white,
            "%s opens the chest",
            actor->name);

        return true;
    }

    game_log(
        actor->floor,
        actor->x,
        actor->y,
        TCOD_white,
        "%s can't open the chest",
        actor->name);

    return false;
}

bool actor_pray(struct actor *actor, int x, int y)
{
    if (!map_is_inside(x, y))
    {
        return false;
    }

    struct map *map = &game->maps[actor->floor];
    struct tile *tile = &map->tiles[x][y];

    if (tile->object && tile->object->type == OBJECT_TYPE_ALTAR)
    {
        game_log(
            actor->floor,
            actor->x,
            actor->y,
            TCOD_white,
            "%s prays at the altar",
            actor->name);

        return true;
    }

    game_log(
        actor->floor,
        actor->x,
        actor->y,
        TCOD_white,
        "%s can't pray here",
        actor->name);

    return false;
}

bool actor_drink(struct actor *actor, int x, int y)
{
    if (!map_is_inside(x, y))
    {
        return false;
    }

    struct map *map = &game->maps[actor->floor];
    struct tile *tile = &map->tiles[x][y];

    if (tile->object && tile->object->type == OBJECT_TYPE_FOUNTAIN)
    {
        int hp = roll(1, 4);
        int max_hp = actor_calc_max_hp(actor);

        actor->current_hp += hp;

        if (actor->current_hp > max_hp)
        {
            actor->current_hp = max_hp;
        }

        game_log(
            actor->floor,
            actor->x,
            actor->y,
            TCOD_white,
            "%s drinks from the fountain, restoring %d health",
            actor->name,
            hp);

        return true;
    }

    game_log(
        actor->floor,
        actor->x,
        actor->y,
        TCOD_white,
        "%s can't drink here",
        actor->name);

    return false;
}

bool actor_sit(struct actor *actor, int x, int y)
{
    if (!map_is_inside(x, y))
    {
        return false;
    }

    struct map *map = &game->maps[actor->floor];
    struct tile *tile = &map->tiles[x][y];

    if (tile->object && tile->object->type == OBJECT_TYPE_THRONE)
    {
        game_log(
            actor->floor,
            actor->x,
            actor->y,
            TCOD_white,
            "%s sits on the throne",
            actor->name);

        return true;
    }

    game_log(
        actor->floor,
        actor->x,
        actor->y,
        TCOD_white,
        "%s can't sit here",
        actor->name);

    return false;
}

bool actor_grab(struct actor *actor, int x, int y)
{
    if (!map_is_inside(x, y))
    {
        return false;
    }

    struct map *map = &game->maps[actor->floor];
    struct tile *tile = &map->tiles[x][y];

    if (TCOD_list_size(tile->items) == 0)
    {
        game_log(
            actor->floor,
            actor->x,
            actor->y,
            TCOD_white,
            "%s cannot find anything to pick up",
            actor->name);

        return false;
    }

    struct item *item = TCOD_list_pop(tile->items);

    item->floor = actor->floor;
    item->x = actor->x;
    item->y = actor->y;

    // TODO: stacking?
    TCOD_list_push(actor->items, item);

    game_log(
        actor->floor,
        actor->x,
        actor->y,
        TCOD_white,
        "%s picks up %s",
        actor->name,
        item_info[item->type].name);

    return true;
}

bool actor_drop(struct actor *actor, struct item *item)
{
    struct map *map = &game->maps[actor->floor];
    struct tile *tile = &map->tiles[actor->x][actor->y];

    item->floor = actor->floor;
    item->x = actor->x;
    item->y = actor->y;

    TCOD_list_push(tile->items, item);
    TCOD_list_remove(actor->items, item);

    game_log(
        actor->floor,
        actor->x,
        actor->y,
        TCOD_white,
        "%s drops %s",
        actor->name,
        item_info[item->type].name);

    return true;
}

bool actor_equip(struct actor *actor, struct item *item)
{
    enum base_item base_item = item_info[item->type].base_item;

    enum equip_slot equip_slot = base_item_info[base_item].equip_slot;

    if (equip_slot == EQUIP_SLOT_NONE)
    {
        game_log(
            actor->floor,
            actor->x,
            actor->y,
            TCOD_white,
            "%s cannot equip %s",
            actor->name,
            item_info[item->type].name);

        return false;
    }

    enum weapon_size weapon_size = base_item_info[base_item].weapon_size;
    enum race_size race_size = race_info[actor->race].size;

    if (weapon_size == WEAPON_SIZE_LARGE && race_size == RACE_SIZE_SMALL)
    {
        game_log(
            actor->floor,
            actor->x,
            actor->y,
            TCOD_white,
            "%s is too large for %s to wield",
            item_info[item->type].name,
            actor->name);

        return false;
    }

    if (actor->equipment[equip_slot])
    {
        actor_unequip(actor, equip_slot);
    }

    if (item_is_two_handed(item, race_size))
    {
        struct item *off_hand = actor->equipment[EQUIP_SLOT_OFF_HAND];

        if (off_hand)
        {
            actor_unequip(actor, EQUIP_SLOT_OFF_HAND);
        }
    }

    if (equip_slot == EQUIP_SLOT_OFF_HAND)
    {
        struct item *main_hand = actor->equipment[EQUIP_SLOT_MAIN_HAND];

        if (main_hand && item_is_two_handed(main_hand, race_size))
        {
            actor_unequip(actor, EQUIP_SLOT_MAIN_HAND);
        }
    }

    TCOD_list_remove(actor->items, item);
    actor->equipment[equip_slot] = item;

    game_log(
        actor->floor,
        actor->x,
        actor->y,
        TCOD_white,
        "%s equips %s",
        actor->name,
        item_info[item->type].name);

    return true;
}

bool actor_unequip(struct actor *actor, enum equip_slot equip_slot)
{
    struct item *equipment = actor->equipment[equip_slot];

    if (!equipment)
    {
        game_log(
            actor->floor,
            actor->x,
            actor->y,
            TCOD_white,
            "%s is not equipping anything their %s slot",
            actor->name,
            equip_slot_info[equip_slot].name);

        return false;
    }

    TCOD_list_push(actor->items, equipment);
    actor->equipment[equip_slot] = NULL;

    game_log(
        actor->floor,
        actor->x,
        actor->y,
        TCOD_white,
        "%s unequips %s",
        actor->name,
        item_info[equipment->type].name);

    return true;
}

bool actor_quaff(struct actor *actor, struct item *item)
{
    if (item_info[item->type].base_item != BASE_ITEM_POTION)
    {
        game_log(
            actor->floor,
            actor->x,
            actor->y,
            TCOD_white,
            "%s cannot quaff %s",
            actor->name,
            item_info[item->type].name);

        return false;
    }

    // TODO: cast the spell stored in the potion

    game_log(
        actor->floor,
        actor->x,
        actor->y,
        TCOD_white,
        "%s quaffs %s",
        actor->name,
        item_info[item->type].name);

    return true;
}

bool actor_bash(struct actor *actor, struct object *object)
{
    if (object->type == OBJECT_TYPE_STAIR_DOWN || object->type == OBJECT_TYPE_STAIR_UP)
    {
        game_log(
            actor->floor,
            actor->x,
            actor->y,
            TCOD_white,
            "%s cannot destroy the %s",
            actor->name,
            object_info[object->type].name);

        return false;
    }

    object->destroyed = true;

    game_log(
        actor->floor,
        actor->x,
        actor->y,
        TCOD_white,
        "%s destroys the %s",
        actor->name,
        object_info[object->type].name);

    return true;
}

bool actor_swing(struct actor *actor, int x, int y)
{
    if (!map_is_inside(x, y))
    {
        return false;
    }

    struct map *map = &game->maps[actor->floor];
    struct tile *tile = &map->tiles[x][y];

    bool hit = false;

    if (tile->actor && tile->actor != actor)
    {
        hit = true;

        if (actor_attack(actor, tile->actor, false))
        {
            return true;
        }
    }

    if (tile->object)
    {
        hit = true;

        if (actor_bash(actor, tile->object))
        {
            return true;
        }
    }

    if (!hit)
    {
        game_log(
            actor->floor,
            actor->x,
            actor->y,
            TCOD_white,
            "%s swings at the air!",
            actor->name);
    }

    return true;
}

bool actor_shoot(struct actor *actor, int x, int y, void(*on_hit)(void *on_hit_params), void *on_hit_params)
{
    if (x == actor->x && y == actor->y)
    {
        return false;
    }

    struct map *map = &game->maps[actor->floor];
    struct item *weapon = actor->equipment[EQUIP_SLOT_MAIN_HAND];

    if (!weapon)
    {
        game_log(
            actor->floor,
            actor->x,
            actor->y,
            TCOD_white,
            "%s cannot shoot without a weapon!",
            actor->name);

        return false;
    }

    enum base_item base_item = item_info[weapon->type].base_item;

    if (!base_item_info[base_item].ranged)
    {
        game_log(
            actor->floor,
            actor->x,
            actor->y,
            TCOD_white,
            "%s cannot shoot without a ranged weapon!",
            actor->name);

        return false;
    }

    struct projectile *projectile = projectile_create(
        '`',
        actor->floor,
        actor->x,
        actor->y,
        x,
        y,
        actor,
        on_hit,
        on_hit_params);

    TCOD_list_push(map->projectiles, projectile);

    return true;
}

bool actor_attack(struct actor *actor, struct actor *other, bool ranged)
{
    if (other->dead)
    {
        game_log(
            actor->floor,
            actor->x,
            actor->y,
            TCOD_white,
            "%s cannot attack that!",
            actor->name);

        return false;
    }

    // TODO: support dual-wield weapons
    // should just attack with both weapons in one move, applying relevant penalties

    // TODO: is attack and damage affected by two-handedness?

    int attack_roll = roll(1, 20);
    int attack_bonus = actor_calc_attack_bonus(actor);
    int total_attack = attack_roll + attack_bonus;
    int armor_class = actor_calc_armor_class(other);
    bool hit = attack_roll == 1
        ? false
        : attack_roll == 20
        ? true
        : total_attack >= armor_class;

    if (hit)
    {
        int num_dice;
        int die_to_roll;
        int crit_threat;
        int crit_mult;
        actor_calc_weapon(actor, &num_dice, &die_to_roll, &crit_threat, &crit_mult, ranged);

        int DAMAGE_TYPE_rolls = 1;

        bool crit = false;
        if (attack_roll >= crit_threat)
        {
            int threat_roll = roll(1, 20);
            int total_threat = threat_roll + attack_bonus;

            if (total_threat >= armor_class)
            {
                crit = true;
                DAMAGE_TYPE_rolls *= crit_mult;
            }
        }

        int total_damage = 0;
        int damage_bonus = actor_calc_damage_bonus(actor);
        for (int i = 0; i < DAMAGE_TYPE_rolls; i++)
        {
            int DAMAGE_TYPE_roll = roll(num_dice, die_to_roll);
            int damage = DAMAGE_TYPE_roll + damage_bonus;

            total_damage += damage;
        }

        game_log(
            actor->floor,
            actor->x,
            actor->y,
            crit ? TCOD_yellow : TCOD_white,
            "%s %s %s for %d",
            actor->name,
            crit ? "crits" : "hits",
            other->name,
            total_damage);

        other->current_hp -= total_damage;
        other->flash_color = TCOD_red;
        other->flash_fade = 1.0f;

        if (other->current_hp <= 0)
        {
            other->current_hp = 0;

            actor_die(other, actor);
        }
    }
    else
    {
        switch (TCOD_random_get_int(NULL, 0, 2))
        {
        case 0:
        {
            game_log(
                actor->floor,
                actor->x,
                actor->y,
                TCOD_grey,
                "%s is parried by %s",
                actor->name,
                other->name);
        }
        break;
        case 1:
        {
            game_log(
                actor->floor,
                actor->x,
                actor->y,
                TCOD_grey,
                "%s is blocked by %s",
                actor->name,
                other->name);
        }
        break;
        case 2:
        {
            game_log(
                actor->floor,
                actor->x,
                actor->y,
                TCOD_grey,
                "%s misses %s",
                actor->name,
                other->name);
        }
        break;
        }
    }

    return true;
}

bool actor_cast_spell(struct actor *actor)
{
    (void)actor;

    return false;
}

void actor_die(struct actor *actor, struct actor *killer)
{
    struct map *map = &game->maps[actor->floor];
    struct tile *tile = &map->tiles[actor->x][actor->y];

    actor->dead = true;

    if (actor != game->player)
    {
        actor->glow = false;
        actor->torch = false;

        for (int i = 0; i < NUM_EQUIP_SLOTS; i++)
        {
            struct item *equipment = actor->equipment[i];

            if (equipment)
            {
                TCOD_list_push(actor->items, equipment);
                actor->equipment[i] = NULL;
            }
        }

        TCOD_LIST_FOREACH(actor->items)
        {
            struct item *item = *iterator;

            TCOD_list_push(tile->items, item);

            iterator = TCOD_list_remove_iterator(actor->items, iterator);
        }
    }

    game_log(
        actor->floor,
        actor->x,
        actor->y,
        TCOD_red,
        "%s dies",
        actor->name);

    if (killer)
    {
        int experience = TCOD_random_get_int(NULL, 50, 100) * actor->level;

        killer->experience += experience;
        killer->kills++;

        game_log(
            killer->floor,
            killer->x,
            killer->y,
            TCOD_azure,
            "%s gains %d experience",
            killer->name,
            experience);
    }

    if (actor == game->player)
    {
        game_log(
            actor->floor,
            actor->x,
            actor->y,
            TCOD_green,
            "Game over! Press 'ESC' to return to the menu");
    }
}

void actor_destroy(struct actor *actor)
{
    if (actor->fov != NULL)
    {
        TCOD_map_delete(actor->fov);
    }

    if (actor->glow_fov != NULL)
    {
        TCOD_map_delete(actor->glow_fov);
    }

    if (actor->torch_fov != NULL)
    {
        TCOD_map_delete(actor->torch_fov);
    }

    TCOD_list_delete(actor->items);

    free(actor->name);

    free(actor);
}

static int calc_ability_modifier(int ability)
{
    return (ability - 10) / 2;
}
