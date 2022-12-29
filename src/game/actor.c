#include "actor.h"

#include "assets.h"
#include "explosion.h"
#include "item.h"
#include "projectile.h"
#include "room.h"
#include "util.h"
#include "world.h"
#include <assert.h>
#include <float.h>
#include <malloc.h>
#include <math.h>

struct actor *actor_new(
    const char *const name,
    const enum race race,
    const enum class class,
    const enum faction faction,
    const uint8_t floor,
    const uint8_t x,
    const uint8_t y)
{
    struct actor *const actor = malloc(sizeof(*actor));
    assert(actor);

    actor->name = TCOD_strdup(name);
    actor->race = race;
    actor->class = class;
    actor->faction = faction;

    actor->level = 1;
    actor->experience = 0;
    actor->ability_points = 0;

    for (enum ability ability = 0; ability < NUM_ABILITIES; ability++)
    {
        actor->ability_scores[ability] = 10;
    }

    actor->base_hit_points = TCOD_random_dice_new(class_data[actor->class].hit_die).nb_faces;
    actor->base_mana_points = TCOD_random_dice_new(class_data[actor->class].mana_die).nb_faces;

    actor->hit_points = actor_calc_max_hit_points(actor);
    actor->mana_points = actor_calc_max_mana_points(actor);

    actor->gold = 0;
    for (enum equip_slot equip_slot = 0; equip_slot < NUM_EQUIP_SLOTS; equip_slot++)
    {
        actor->equipment[equip_slot] = NULL;
    }
    actor->items = TCOD_list_new();
    actor->known_spell_types = TCOD_list_new();
    actor->readied_spell_type = SPELL_TYPE_NONE;

    actor->floor = floor;
    actor->x = x;
    actor->y = y;

    actor->fov = NULL;

    actor->took_turn = false;
    actor->energy = actor_calc_speed(actor);

    actor->last_seen_x = -1;
    actor->last_seen_y = -1;
    actor->turns_chased = 0;

    actor->leader = NULL;

    actor->light_type = LIGHT_TYPE_NONE;
    actor->light_fov = NULL;

    actor->flash_color = TCOD_white;
    actor->flash_fade_coef = 0.0f;

    actor->controllable = false;

    actor->dead = false;

    return actor;
}

void actor_delete(struct actor *const actor)
{
    if (actor->light_fov != NULL)
    {
        TCOD_map_delete(actor->light_fov);
    }

    if (actor->fov != NULL)
    {
        TCOD_map_delete(actor->fov);
    }

    TCOD_list_delete(actor->known_spell_types);

    TCOD_LIST_FOREACH(actor->items)
    {
        item_delete(*iterator);
    }
    TCOD_list_delete(actor->items);

    free(actor->name);

    free(actor);
}

int actor_calc_experience_for_level(const int level)
{
    return level * (level - 1) / 2 * 1000;
}

int actor_calc_ability_modifer(const struct actor *const actor, const enum ability ability)
{
    return (actor->ability_scores[ability] - 10) / 2;
}

int actor_calc_max_hit_points(const struct actor *const actor)
{
    return actor->base_hit_points + (actor->level * actor_calc_ability_modifer(actor, ABILITY_CONSTITUTION));
}

int actor_calc_max_mana_points(const struct actor *const actor)
{
    return actor->base_mana_points + (actor->level * actor_calc_ability_modifer(actor, ABILITY_INTELLIGENCE));
}

int actor_calc_armor_class(const struct actor *const actor)
{
    const int dexterity_modifer = actor_calc_ability_modifer(actor, ABILITY_DEXTERITY);
    const int size_modifer = size_data[race_data[actor->race].size].modifier;

    int armor_class = 10 + dexterity_modifer + size_modifer;

    const struct item *const armor = actor->equipment[EQUIP_SLOT_ARMOR];
    if (armor)
    {
        const struct item_datum *const armor_datum = &item_data[armor->type];
        const struct base_item_datum *const base_armor_datum = &base_item_data[armor_datum->type];

        armor_class += base_armor_datum->armor_class;
        armor_class += armor_datum->enhancement_bonus;
    }

    const struct item *const shield = actor->equipment[EQUIP_SLOT_SHIELD];
    if (shield)
    {
        const struct item_datum *const shield_datum = &item_data[shield->type];
        const struct base_item_datum *const base_shield_datum = &base_item_data[shield_datum->type];

        armor_class += base_shield_datum->armor_class;
        armor_class += shield_datum->enhancement_bonus;
    }

    return armor_class;
}

int actor_calc_base_attack_bonus(const struct actor *actor)
{
    return (actor->level - 1) * 1;
}

int actor_calc_attack_bonus(const struct actor *const actor)
{
    const int base_attack_bonus = actor_calc_base_attack_bonus(actor);
    const int size_modifer = size_data[race_data[actor->race].size].modifier;

    int attack_bonus = base_attack_bonus + size_modifer;

    const struct item *const weapon = actor->equipment[EQUIP_SLOT_WEAPON];
    if (weapon)
    {
        const struct item_datum *const weapon_datum = &item_data[weapon->type];

        attack_bonus += weapon_datum->enhancement_bonus;

        const struct base_item_datum *const base_weapon_datum = &base_item_data[weapon_datum->type];
        if (base_weapon_datum->ranged)
        {
            attack_bonus += actor_calc_ability_modifer(actor, ABILITY_DEXTERITY);

            const struct item *const ammunition = actor->equipment[EQUIP_SLOT_AMMUNITION];
            if (ammunition)
            {
                const struct item_datum *const ammunition_datum = &item_data[ammunition->type];

                attack_bonus += ammunition_datum->enhancement_bonus;
            }
        }
        else
        {
            if (actor->class == CLASS_ROGUE)
            {
                attack_bonus += actor_calc_ability_modifer(actor, ABILITY_DEXTERITY);
            }
            else
            {
                attack_bonus += actor_calc_ability_modifer(actor, ABILITY_STRENGTH);
            }
        }
    }
    else
    {
        attack_bonus += actor_calc_ability_modifer(actor, ABILITY_STRENGTH);
    }

    return attack_bonus;
}

int actor_calc_threat_range(const struct actor *const actor)
{
    const struct item *const weapon = actor->equipment[EQUIP_SLOT_WEAPON];
    if (weapon)
    {
        const struct item_datum *const weapon_datum = &item_data[weapon->type];
        const struct base_item_datum *const base_weapon_datum = &base_item_data[weapon_datum->type];

        return base_weapon_datum->threat_range;
    }

    return 20;
}

int actor_calc_critical_multiplier(const struct actor *const actor)
{
    const struct item *const weapon = actor->equipment[EQUIP_SLOT_WEAPON];
    if (weapon)
    {
        const struct item_datum *const weapon_datum = &item_data[weapon->type];
        const struct base_item_datum *const base_weapon_datum = &base_item_data[weapon_datum->type];

        return base_weapon_datum->critical_multiplier;
    }

    return 2;
}

int actor_calc_damage_bonus(const struct actor *const actor)
{
    int damage_bonus = 0;

    const struct item *const weapon = actor->equipment[EQUIP_SLOT_WEAPON];
    if (weapon)
    {
        const struct item_datum *const weapon_datum = &item_data[weapon->type];

        damage_bonus += weapon_datum->enhancement_bonus;

        const struct base_item_datum *const base_weapon_datum = &base_item_data[weapon_datum->type];
        if (base_weapon_datum->ranged)
        {
            const int strength_modifier = actor_calc_ability_modifer(actor, ABILITY_STRENGTH);

            if (weapon_datum->type == BASE_ITEM_TYPE_SLING)
            {
                damage_bonus += strength_modifier;
            }
            else if (strength_modifier < 0) // TODO: if composite bows are added, then exclude them from the strength penalty
            {
                damage_bonus += strength_modifier;
            }

            const struct item *const ammunition = actor->equipment[EQUIP_SLOT_AMMUNITION];
            if (ammunition)
            {
                const struct item_datum *const ammunition_datum = &item_data[ammunition->type];

                damage_bonus += ammunition_datum->enhancement_bonus;
            }
        }
        else
        {
            const enum equippability equippability = actor_calc_item_equippability(actor, weapon);

            if (equippability == EQUIPPABILITY_BARELY)
            {
                damage_bonus += (int)(actor_calc_ability_modifer(actor, ABILITY_STRENGTH) * 1.5f);
            }
            else
            {
                damage_bonus += actor_calc_ability_modifer(actor, ABILITY_STRENGTH);
            }
        }
    }
    else
    {
        damage_bonus += actor_calc_ability_modifer(actor, ABILITY_STRENGTH);
    }

    return damage_bonus;
}

const char *actor_calc_damage(const struct actor *const actor)
{
    const struct item *const weapon = actor->equipment[EQUIP_SLOT_WEAPON];
    if (weapon)
    {
        const struct item_datum *const weapon_datum = &item_data[weapon->type];
        const struct base_item_datum *const base_weapon_datum = &base_item_data[weapon_datum->type];

        return base_weapon_datum->damage;
    }

    return "1d3";
}

enum equippability actor_calc_item_equippability(const struct actor *const actor, const struct item *const item)
{
    const struct race_datum *const race_datum = &race_data[actor->race];
    const struct item_datum *const item_datum = &item_data[item->type];
    const struct base_item_datum *const base_item_datum = &base_item_data[item_datum->type];

    const int difference = race_datum->size - base_item_datum->size;

    if (difference < -1)
    {
        return EQUIPPABILITY_TOO_LARGE;
    }
    else if (difference == -1)
    {
        return EQUIPPABILITY_BARELY;
    }
    if (difference == 0)
    {
        return EQUIPPABILITY_COMFORTABLY;
    }
    if (difference == 1 || difference == 2)
    {
        return EQUIPPABILITY_EASILY;
    }
    else
    {
        return EQUIPPABILITY_TOO_SMALL;
    }
}

float actor_calc_speed(const struct actor *actor)
{
    const float speed = race_data[actor->race].speed;

    float encumbrance = 1.0f;

    for (enum equip_slot equip_slot = 1; equip_slot < NUM_EQUIP_SLOTS; equip_slot++)
    {
        if (actor->equipment[equip_slot])
        {
            enum equippability equippability = actor_calc_item_equippability(actor, actor->equipment[equip_slot]);

            if (equippability == EQUIPPABILITY_BARELY)
            {
                encumbrance -= 0.2f;
            }
        }
    }

    return speed * encumbrance;
}

void actor_calc_light(struct actor *const actor)
{
    if (actor->light_fov)
    {
        TCOD_map_delete(actor->light_fov);
        actor->light_fov = NULL;
    }

    const struct light_datum *const light_datum = &light_data[actor->light_type];

    if (light_datum->radius >= 0)
    {
        actor->light_fov = map_to_fov_map(
            &world->maps[actor->floor],
            actor->x,
            actor->y,
            light_datum->radius);
    }
}

void actor_calc_fade(struct actor *const actor, const float delta_time)
{
    if (actor->flash_fade_coef > 0)
    {
        // TODO: slower/faster fade depending on circumstances
        // TODO: different fade functions such as sin()
        actor->flash_fade_coef -= 4.0f * delta_time;
    }
    else
    {
        actor->flash_fade_coef = 0.0f;
    }
}

int actor_calc_sight_radius(struct actor *actor)
{
    if (actor->race == RACE_ELF)
    {
        return 3;
    }

    return 1;
}

void actor_calc_fov(struct actor *const actor)
{
    if (actor->fov)
    {
        TCOD_map_delete(actor->fov);
    }

    const struct map *const map = &world->maps[actor->floor];
    actor->fov = map_to_fov_map(map, actor->x, actor->y, actor_calc_sight_radius(actor));

    TCOD_map_t los_map = map_to_fov_map(map, actor->x, actor->y, 0);

    for (int x = 0; x < MAP_WIDTH; x++)
    {
        for (int y = 0; y < MAP_HEIGHT; y++)
        {
            if (!TCOD_map_is_in_fov(actor->fov, x, y) &&
                TCOD_map_is_in_fov(los_map, x, y))
            {
                TCOD_LIST_FOREACH(map->objects)
                {
                    const struct object *const object = *iterator;
                    if (object->light_fov &&
                        TCOD_map_is_in_fov(object->light_fov, x, y))
                    {
                        TCOD_map_set_in_fov(actor->fov, x, y, true);
                    }
                }

                TCOD_LIST_FOREACH(map->actors)
                {
                    const struct actor *const other = *iterator;
                    if (other->light_fov &&
                        TCOD_map_is_in_fov(other->light_fov, x, y))
                    {
                        TCOD_map_set_in_fov(actor->fov, x, y, true);
                    }
                }

                TCOD_LIST_FOREACH(map->projectiles)
                {
                    const struct projectile *const projectile = *iterator;
                    if (projectile->light_fov &&
                        TCOD_map_is_in_fov(projectile->light_fov, x, y))
                    {
                        TCOD_map_set_in_fov(actor->fov, x, y, true);
                    }
                }

                TCOD_LIST_FOREACH(map->explosions)
                {
                    const struct explosion *const explosion = *iterator;
                    if (explosion->fov &&
                        TCOD_map_is_in_fov(explosion->fov, x, y))
                    {
                        TCOD_map_set_in_fov(actor->fov, x, y, true);
                    }
                }
            }
        }
    }

    TCOD_map_delete(los_map);
}

void actor_give_experience(struct actor *const actor, const int experience)
{
    actor->experience += experience;

    world_log(
        actor->floor,
        actor->x,
        actor->y,
        TCOD_azure,
        "%s gains %d experience.",
        actor->name,
        experience);

    while (actor->experience >= actor_calc_experience_for_level(actor->level + 1))
    {
        actor_level_up(actor);

        world_log(
            actor->floor,
            actor->x,
            actor->y,
            TCOD_yellow,
            "%s has gained a level!",
            actor->name);
    }
}

void actor_level_up(struct actor *const actor)
{
    const float current_hit_points_percent = (float)actor->hit_points / actor_calc_max_hit_points(actor);
    const float current_mana_points_percent = (float)actor->mana_points / actor_calc_max_mana_points(actor);

    actor->level++;
    actor->ability_points++;
    actor->base_hit_points += TCOD_random_dice_roll_s(world->random, class_data[actor->class].hit_die);
    actor->base_mana_points += TCOD_random_dice_roll_s(world->random, class_data[actor->class].mana_die);

    actor->hit_points = (int)(actor_calc_max_hit_points(actor) * current_hit_points_percent);
    actor->mana_points = (int)(actor_calc_max_mana_points(actor) * current_mana_points_percent);

    if (actor->hit_points <= 0)
    {
        actor_die(actor, NULL);
    }
    if (actor->mana_points <= 0)
    {
        actor->mana_points = 0;
    }
}

void actor_add_ability_point(struct actor *const actor, const enum ability ability)
{
    const float current_hit_points_percent = (float)actor->hit_points / actor_calc_max_hit_points(actor);
    const float current_mana_points_percent = (float)actor->mana_points / actor_calc_max_mana_points(actor);

    actor->ability_scores[ability]++;
    actor->ability_points--;

    actor->hit_points = (int)(actor_calc_max_hit_points(actor) * current_hit_points_percent);
    actor->mana_points = (int)(actor_calc_max_mana_points(actor) * current_mana_points_percent);

    if (actor->hit_points <= 0)
    {
        actor_die(actor, NULL);
    }
    if (actor->mana_points <= 0)
    {
        actor->mana_points = 0;
    }
}

bool actor_can_take_turn(const struct actor *const actor)
{
    return actor->energy >= 1.0f && !actor->dead;
}

struct actor *actor_find_closest_enemy(const struct actor *const actor)
{
    struct actor *closest_enemy = NULL;

    float min_distance = FLT_MAX;
    struct map *map = &world->maps[world->player->floor];
    TCOD_LIST_FOREACH(map->actors)
    {
        struct actor *const other = *iterator;
        if (TCOD_map_is_in_fov(actor->fov, other->x, other->y) &&
            other->faction != actor->faction)
        {
            const float distance = distance_between_sq(
                actor->x, actor->y,
                other->x, other->y);
            if (distance < min_distance)
            {
                closest_enemy = other;
                min_distance = distance;
            }
        }
    }

    return closest_enemy;
}

bool actor_ai(struct actor *const actor)
{
    while (actor->ability_points > 0)
    {
        const enum ability ability = TCOD_random_get_int(world->random, 0, NUM_ABILITIES - 1);

        actor_add_ability_point(actor, ability);
    }

    const struct map *const map = &world->maps[actor->floor];
    const struct tile *const tile = &map->tiles[actor->x][actor->y];

    // look for fountains to heal if low health
    if (actor->hit_points < actor_calc_max_hit_points(actor) / 2)
    {
        const struct object *target = NULL;

        float min_distance = FLT_MAX;
        TCOD_LIST_FOREACH(map->objects)
        {
            const struct object *const object = *iterator;
            if (TCOD_map_is_in_fov(actor->fov, object->x, object->y) &&
                object->type == OBJECT_TYPE_FOUNTAIN)
            {
                const float distance = distance_between_sq(
                    actor->x, actor->y,
                    object->x, object->y);
                if (distance < min_distance)
                {
                    target = object;
                    min_distance = distance;
                }
            }
        }

        if (target)
        {
            if (distance_between(
                    actor->x, actor->y,
                    target->x, target->y) < 2.0f)
            {
                if (actor_drink(actor, target->x, target->y))
                {
                    return true;
                }
            }
            else
            {
                if (actor_path_towards(actor, target->x, target->y))
                {
                    return true;
                }
            }
        }
    }

    // look for hostile targets
    {
        struct actor *target = actor_find_closest_enemy(actor);

        if (target)
        {
            // target spotted, so remember the location in case the actor loses them
            actor->last_seen_x = target->x;
            actor->last_seen_y = target->y;
            actor->turns_chased = 0;

            bool ranged = false;

            const struct item *const weapon = actor->equipment[EQUIP_SLOT_WEAPON];
            if (weapon)
            {
                const struct item_datum *const weapon_datum = &item_data[weapon->type];
                const struct base_item_datum *const base_weapon_datum = &base_item_data[weapon_datum->type];

                if (base_weapon_datum->ranged)
                {
                    // does the actor have ammo?
                    struct item *ammunition = actor->equipment[EQUIP_SLOT_AMMUNITION];
                    if (ammunition)
                    {
                        // is it the correct ammo?
                        const struct item_datum *const ammunition_datum = &item_data[ammunition->type];
                        const struct base_item_datum *const base_ammunition_datum = &base_item_data[ammunition_datum->type];
                        if (base_weapon_datum->ammunition_type == base_ammunition_datum->ammunition_type)
                        {
                            ranged = true;
                        }
                    }
                    else
                    {
                        // out of ammo or using the wrong ammo, so unequip the weapon
                        // TODO: look in inventory for suitable ammo and equip
                        if (actor_unequip(actor, EQUIP_SLOT_WEAPON))
                        {
                            return true;
                        }
                    }
                }
            }

            if (ranged)
            {
                if (actor_shoot(actor, target->x, target->y))
                {
                    return true;
                }
            }
            else
            {
                if (distance_between(
                        actor->x, actor->y,
                        target->x, target->y) < 2.0f)
                {
                    if (actor_attack(actor, target, NULL))
                    {
                        return true;
                    }
                }
                else
                {
                    if (actor_path_towards(actor, target->x, target->y))
                    {
                        return true;
                    }
                }
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

            return true;
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
                return true;
            }
        }
    }

    // TODO: resting

    // TODO: look for objects to interact with if needed (fountains already done, but consider other types)

    // TODO: look for items to pick up

    // pick up items on ground
    if (TCOD_list_size(tile->items) > 0)
    {
        if (actor_grab(actor, actor->x, actor->y))
        {
            return true;
        }
    }

    // move randomly
    if (TCOD_random_get_int(world->random, 0, 1) == 0)
    {
        const int x = actor->x + TCOD_random_get_int(world->random, -1, 1);
        const int y = actor->y + TCOD_random_get_int(world->random, -1, 1);
        actor_move(actor, x, y);

        return true;
    }

    return true;
}

bool actor_rest(struct actor *const actor)
{
    actor_restore_hit_points(actor, 1);
    actor_restore_mana_points(actor, 1);

    return true;
}

bool actor_path_towards(
    struct actor *const actor,
    const int target_x, const int target_y)
{
    TCOD_map_t TCOD_map = map_to_TCOD_map(&world->maps[actor->floor]);
    TCOD_map_set_properties(
        TCOD_map,
        target_x, target_y,
        TCOD_map_is_transparent(TCOD_map, target_x, target_y),
        true);

    TCOD_path_t path = TCOD_path_new_using_map(TCOD_map, 1.0f);
    TCOD_path_compute(
        path,
        actor->x, actor->y,
        target_x, target_y);

    bool success = false;

    int next_x, next_y;
    if (!TCOD_path_is_empty(path) &&
        TCOD_path_walk(path, &next_x, &next_y, false))
    {
        success = actor_move(actor, next_x, next_y);
    }
    else
    {
        success = actor_move_towards(actor, target_x, target_y);
    }

    TCOD_path_delete(path);
    TCOD_map_delete(TCOD_map);

    return success;
}

bool actor_move_towards(
    struct actor *const actor,
    const int target_x, const int target_y)
{
    int dx = target_x - actor->x;
    int dy = target_y - actor->y;
    const float distance = distance_between(
        actor->x, actor->y,
        target_x, target_y);
    if (distance > 0.0f)
    {
        dx = (int)round(dx / distance);
        dy = (int)round(dy / distance);

        return actor_move(
            actor,
            actor->x + dx, actor->y + dy);
    }

    return false;
}

bool actor_move(
    struct actor *const actor,
    const int x, const int y)
{
    if (!map_is_inside(x, y))
    {
        return false;
    }

    struct map *const map = &world->maps[actor->floor];
    struct tile *const tile = &map->tiles[x][y];

    if (!tile_data[tile->type].is_walkable)
    {
        return false;
    }

    if (tile->object)
    {
        const enum object_type object_type = tile->object->type;

        switch (object_type)
        {
        case OBJECT_TYPE_ALTAR:
        {
            return actor_pray(actor, x, y);
        }
        break;
        case OBJECT_TYPE_BRAZIER:
        {
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
        case OBJECT_TYPE_DOOR_OPEN:
        {
        }
        break;
        case OBJECT_TYPE_FOUNTAIN:
        {
            return actor_drink(actor, x, y);
        }
        break;
        case OBJECT_TYPE_STAIR_DOWN:
        {
        }
        break;
        case OBJECT_TYPE_STAIR_UP:
        {
        }
        break;
        case OBJECT_TYPE_THRONE:
        {
            return actor_sit(actor, x, y);
        }
        break;
        case OBJECT_TYPE_TRAP:
        {
            // TODO: trap effects

            TCOD_list_remove(map->objects, tile->object);

            object_delete(tile->object);
            tile->object = NULL;

            world_log(
                actor->floor,
                actor->x,
                actor->y,
                TCOD_white,
                "%s triggers a trap!",
                actor->name);
        }
        break;
        case NUM_OBJECT_TYPES:
            break;
        }

        if (!object_data[object_type].is_walkable)
        {
            return false;
        }
    }

    if (tile->actor && tile->actor != actor)
    {
        if (tile->actor->faction == actor->faction)
        {
            return actor_swap(actor, tile->actor);
        }
        else
        {
            bool ranged = false;

            const struct item *const weapon = actor->equipment[EQUIP_SLOT_WEAPON];
            if (weapon)
            {
                const struct item_datum *const weapon_datum = &item_data[weapon->type];
                const struct base_item_datum *const base_weapon_datum = &base_item_data[weapon_datum->type];

                ranged = base_weapon_datum->ranged;
            }

            if (ranged)
            {
                return actor_shoot(world->player, x, y);
            }
            else
            {
                return actor_attack(actor, tile->actor, NULL);
            }
        }
    }

    struct tile *const current_tile = &map->tiles[actor->x][actor->y];
    current_tile->actor = NULL;

    tile->actor = actor;

    actor->x = (uint8_t)x;
    actor->y = (uint8_t)y;

    return true;
}

bool actor_swap(struct actor *const actor, struct actor *const other)
{
    // npc actors can't initiate a swap with the player
    if (other == world->player)
    {
        return false;
    }

    // swap actor coordinates
    const uint8_t temp_x = actor->x;
    const uint8_t temp_y = actor->y;
    actor->x = other->x;
    actor->y = other->y;
    other->x = temp_x;
    other->y = temp_y;

    // swap actor pointers in the tiles
    struct map *const map = &world->maps[actor->floor];
    map->tiles[actor->x][actor->y].actor = actor;
    map->tiles[other->x][other->y].actor = other;

    world_log(
        actor->floor,
        actor->x,
        actor->y,
        TCOD_white,
        "%s swaps with %s.",
        actor->name,
        other->name);

    return true;
}

bool actor_open_door(
    struct actor *const actor,
    const int x, const int y)
{
    if (!map_is_inside(x, y))
    {
        return false;
    }

    struct map *const map = &world->maps[actor->floor];
    struct tile *const tile = &map->tiles[x][y];
    if (!tile->object || tile->object->type != OBJECT_TYPE_DOOR_CLOSED)
    {
        world_log(
            actor->floor,
            actor->x,
            actor->y,
            TCOD_white,
            "%s can't open the door.",
            actor->name);

        return false;
    }

    tile->object->type = OBJECT_TYPE_DOOR_OPEN;

    world_log(
        actor->floor,
        actor->x,
        actor->y,
        TCOD_orange,
        "%s opens the door.",
        actor->name);

    return true;
}

bool actor_close_door(
    struct actor *const actor,
    const int x, const int y)
{
    if (!map_is_inside(x, y))
    {
        return false;
    }

    struct map *const map = &world->maps[actor->floor];
    struct tile *const tile = &map->tiles[x][y];
    if (!tile->object || tile->object->type != OBJECT_TYPE_DOOR_OPEN)
    {
        world_log(
            actor->floor,
            actor->x,
            actor->y,
            TCOD_white,
            "%s can't close the door.",
            actor->name);

        return false;
    }

    tile->object->type = OBJECT_TYPE_DOOR_CLOSED;

    world_log(
        actor->floor,
        actor->x,
        actor->y,
        TCOD_orange,
        "%s closes the door.",
        actor->name);

    return true;
}

bool actor_descend(
    struct actor *const actor,
    const bool is_leader,
    void ***const iterator)
{
    // is there a next map?
    if (actor->floor >= NUM_MAPS - 1)
    {
        world_log(
            actor->floor,
            actor->x,
            actor->y,
            TCOD_white,
            "%s has reached the end.",
            actor->name);

        return false;
    }

    // is the actor on stairs down?
    struct map *const map = &world->maps[actor->floor];
    struct tile *const tile = &map->tiles[actor->x][actor->y];
    if (is_leader &&
        (!tile->object || tile->object->type != OBJECT_TYPE_STAIR_DOWN))
    {
        world_log(
            actor->floor,
            actor->x,
            actor->y,
            TCOD_white,
            "%s can't descend here.",
            actor->name);

        return false;
    }

    // remove actor from current map
    if (iterator)
    {
        *iterator = TCOD_list_remove_iterator_fast(map->actors, *iterator);
    }
    else
    {
        TCOD_list_remove(map->actors, actor);
    }

    // remove actor from current tile
    tile->actor = NULL;

    // add actor to next map
    struct map *const next_map = &world->maps[actor->floor + 1];
    TCOD_list_push(next_map->actors, actor);

    // add actor to next tile
    struct tile *const next_tile = &next_map->tiles[next_map->stair_up_x][next_map->stair_up_y];
    next_tile->actor = actor;

    // change actor coordinates
    actor->floor = next_map->floor;
    actor->x = next_map->stair_up_x;
    actor->y = next_map->stair_up_y;

    // find all followers and move them to the next map as well
    if (is_leader)
    {
        TCOD_LIST_FOREACH(map->actors)
        {
            struct actor *other = *iterator;
            if (other && other->leader == actor)
            {
                actor_descend(other, false, &iterator);

                if (!iterator)
                {
                    break;
                }
            }
        }
    }

    world_log(
        actor->floor,
        actor->x,
        actor->y,
        TCOD_white,
        "%s descends.",
        actor->name);

    return true;
}

bool actor_ascend(struct actor *actor, bool is_leader, void ***iterator)
{
    // is there a previous map?
    if (actor->floor == 0)
    {
        world_log(
            actor->floor,
            actor->x,
            actor->y,
            TCOD_white,
            "%s can't go any higher.",
            actor->name);

        return false;
    }

    // is the actor on stairs up?
    struct map *map = &world->maps[actor->floor];
    struct tile *tile = &map->tiles[actor->x][actor->y];
    if (is_leader &&
        (!tile->object || tile->object->type != OBJECT_TYPE_STAIR_UP))
    {
        world_log(
            actor->floor,
            actor->x,
            actor->y,
            TCOD_white,
            "%s can't ascend here.",
            actor->name);

        return false;
    }

    // remove actor from current map
    if (iterator)
    {
        *iterator = TCOD_list_remove_iterator_fast(map->actors, *iterator);
    }
    else
    {
        TCOD_list_remove(map->actors, actor);
    }

    // remove actor from current tile
    tile->actor = NULL;

    // add actor to next map
    struct map *const next_map = &world->maps[actor->floor - 1];
    TCOD_list_push(next_map->actors, actor);

    // add actor to next tile
    struct tile *const next_tile = &next_map->tiles[next_map->stair_down_x][next_map->stair_down_y];
    next_tile->actor = actor;

    // change actor coordinates
    actor->floor = next_map->floor;
    actor->x = next_map->stair_down_x;
    actor->y = next_map->stair_down_y;

    // find all followers and move them to the next map as well
    if (is_leader)
    {
        TCOD_LIST_FOREACH(map->actors)
        {
            struct actor *other = *iterator;
            if (other && other->leader == actor)
            {
                actor_ascend(other, false, &iterator);

                if (!iterator)
                {
                    break;
                }
            }
        }
    }

    world_log(
        actor->floor,
        actor->x,
        actor->y,
        TCOD_white,
        "%s ascends.",
        actor->name);

    return true;
}

bool actor_open_chest(
    struct actor *const actor,
    const int x, const int y)
{
    if (!map_is_inside(x, y))
    {
        return false;
    }

    struct map *const map = &world->maps[actor->floor];
    struct tile *const tile = &map->tiles[x][y];
    if (!tile->object || tile->object->type != OBJECT_TYPE_CHEST)
    {
        world_log(
            actor->floor,
            actor->x,
            actor->y,
            TCOD_white,
            "%s can't open the chest.",
            actor->name);

        return false;
    }

    // TODO: give item

    TCOD_list_remove(map->objects, tile->object);

    object_delete(tile->object);
    tile->object = NULL;

    world_log(
        actor->floor,
        actor->x,
        actor->y,
        TCOD_orange,
        "%s opens the chest.",
        actor->name);

    return true;
}

bool actor_pray(
    struct actor *const actor,
    const int x, const int y)
{
    if (!map_is_inside(x, y))
    {
        return false;
    }

    struct map *const map = &world->maps[actor->floor];
    struct tile *const tile = &map->tiles[x][y];
    if (!tile->object || tile->object->type != OBJECT_TYPE_ALTAR)
    {
        world_log(
            actor->floor,
            actor->x,
            actor->y,
            TCOD_white,
            "%s can't pray here.",
            actor->name);

        return false;
    }

    // TODO: prayer effects

    TCOD_list_remove(map->objects, tile->object);

    object_delete(tile->object);
    tile->object = NULL;

    world_log(
        actor->floor,
        actor->x,
        actor->y,
        TCOD_orange,
        "%s prays at the altar.",
        actor->name);

    return true;
}

bool actor_drink(
    struct actor *const actor,
    const int x, const int y)
{
    if (!map_is_inside(x, y))
    {
        return false;
    }

    struct map *const map = &world->maps[actor->floor];
    struct tile *const tile = &map->tiles[x][y];
    if (!tile->object || tile->object->type != OBJECT_TYPE_FOUNTAIN)
    {
        world_log(
            actor->floor,
            actor->x,
            actor->y,
            TCOD_white,
            "%s can't drink here.",
            actor->name);

        return false;
    }

    actor_restore_hit_points(actor, actor_calc_max_hit_points(actor) - actor->hit_points);
    actor_restore_mana_points(actor, actor_calc_max_mana_points(actor) - actor->mana_points);

    TCOD_list_remove(map->objects, tile->object);

    object_delete(tile->object);
    tile->object = NULL;

    world_log(
        actor->floor,
        actor->x,
        actor->y,
        TCOD_orange,
        "%s drinks from the fountain.",
        actor->name);

    return true;
}

bool actor_sit(
    struct actor *const actor,
    const int x, const int y)
{
    if (!map_is_inside(x, y))
    {
        return false;
    }

    struct map *const map = &world->maps[actor->floor];
    struct tile *const tile = &map->tiles[x][y];
    if (!tile->object || tile->object->type != OBJECT_TYPE_THRONE)
    {
        world_log(
            actor->floor,
            actor->x,
            actor->y,
            TCOD_white,
            "%s can't sit here",
            actor->name);

        return false;
    }

    // TODO: throne effects
    // donate gold for a reward?

    TCOD_list_remove(map->objects, tile->object);

    object_delete(tile->object);
    tile->object = NULL;

    world_log(
        actor->floor,
        actor->x,
        actor->y,
        TCOD_orange,
        "%s sits on the throne.",
        actor->name);

    return true;
}

bool actor_grab(
    struct actor *const actor,
    const int x, const int y)
{
    if (!map_is_inside(x, y))
    {
        return false;
    }

    struct map *const map = &world->maps[actor->floor];
    struct tile *const tile = &map->tiles[x][y];
    if (TCOD_list_size(tile->items) == 0)
    {
        world_log(
            actor->floor,
            actor->x,
            actor->y,
            TCOD_white,
            "%s cannot find anything to pick up.",
            actor->name);

        return false;
    }

    if (TCOD_list_size(actor->items) >= 26)
    {
        world_log(
            actor->floor,
            actor->x,
            actor->y,
            TCOD_white,
            "%s is carrying too many items.",
            actor->name);

        return false;
    }

    struct item *const item = TCOD_list_pop(tile->items);

    TCOD_list_remove(map->items, item);

    if (item->type == ITEM_TYPE_GOLD)
    {
        const int gold = item->stack;
        actor->gold += gold;

        item_delete(item);

        world_log(
            actor->floor,
            actor->x,
            actor->y,
            TCOD_white,
            "%s picks up %d gold.",
            actor->name,
            gold);
    }
    else
    {
        TCOD_list_push(actor->items, item);

        world_log(
            actor->floor,
            actor->x,
            actor->y,
            TCOD_white,
            "%s picks up %s.",
            actor->name,
            item_data[item->type].name);
    }

    return true;
}

bool actor_drop(struct actor *const actor, struct item *const item)
{
    // remove from actor's inventory
    TCOD_list_remove(actor->items, item);

    // change item coordinates
    item->floor = actor->floor;
    item->x = actor->x;
    item->y = actor->y;

    // add item to map
    struct map *const map = &world->maps[item->floor];
    TCOD_list_push(map->items, item);

    // add item to tile
    struct tile *const tile = &map->tiles[item->x][item->y];
    TCOD_list_push(tile->items, item);

    world_log(
        actor->floor,
        actor->x,
        actor->y,
        TCOD_white,
        "%s drops %s.",
        actor->name,
        item_data[item->type].name);

    return true;
}

bool actor_equip(struct actor *const actor, struct item *const item)
{
    const struct item_datum *const item_datum = &item_data[item->type];
    const struct base_item_datum *const base_item_datum = &base_item_data[item_datum->type];

    // is item equipment?
    enum equip_slot equip_slot = base_item_datum->equip_slot;
    if (equip_slot == EQUIP_SLOT_NONE)
    {
        world_log(
            actor->floor,
            actor->x,
            actor->y,
            TCOD_white,
            "%s cannot equip %s.",
            actor->name,
            item_datum->name);

        return false;
    }

    // is item equippable
    const enum equippability equippability = actor_calc_item_equippability(actor, item);
    if (equippability == EQUIPPABILITY_TOO_LARGE)
    {
        world_log(
            actor->floor,
            actor->x,
            actor->y,
            TCOD_white,
            "%s is too large for %s.",
            item_datum->name,
            actor->name);

        return false;
    }
    if (equippability == EQUIPPABILITY_TOO_SMALL)
    {
        world_log(
            actor->floor,
            actor->x,
            actor->y,
            TCOD_white,
            "%s is too small for %s.",
            item_datum->name,
            actor->name);

        return false;
    }

    // unequip the current item in the slot
    if (actor->equipment[equip_slot])
    {
        actor_unequip(actor, equip_slot);
    }

    // if the item being equipped is two handed weapon, also unequip the shield
    if (equip_slot == EQUIP_SLOT_WEAPON)
    {
        if (equippability == EQUIPPABILITY_BARELY)
        {
            if (actor->equipment[EQUIP_SLOT_SHIELD])
            {
                actor_unequip(actor, EQUIP_SLOT_SHIELD);
            }
        }
    }

    // if the item being equipped is a shield and the equipped main hand is two handed, also unequip the main hand
    if (equip_slot == EQUIP_SLOT_SHIELD)
    {
        const struct item *const main_hand = actor->equipment[EQUIP_SLOT_WEAPON];

        if (main_hand)
        {
            const enum equippability main_hand_equippability = actor_calc_item_equippability(actor, main_hand);

            if (main_hand_equippability == EQUIPPABILITY_BARELY)
            {
                actor_unequip(actor, EQUIP_SLOT_WEAPON);
            }
        }
    }

    // remove from inventory
    TCOD_list_remove(actor->items, item);

    // add to slot
    actor->equipment[equip_slot] = item;

    world_log(
        actor->floor,
        actor->x,
        actor->y,
        TCOD_white,
        "%s equips %s.",
        actor->name,
        item_datum->name);

    return true;
}

bool actor_unequip(struct actor *const actor, const enum equip_slot equip_slot)
{
    // is something equipped in the slot?
    struct item *equipment = actor->equipment[equip_slot];
    if (!equipment)
    {
        world_log(
            actor->floor,
            actor->x,
            actor->y,
            TCOD_white,
            "%s is not equipping anything their %s slot.",
            actor->name,
            equip_slot_data[equip_slot].name);

        return false;
    }

    // add to inventory
    TCOD_list_push(actor->items, equipment);

    // remove from slot
    actor->equipment[equip_slot] = NULL;

    world_log(
        actor->floor,
        actor->x,
        actor->y,
        TCOD_white,
        "%s unequips %s.",
        actor->name,
        item_data[equipment->type].name);

    return true;
}

bool actor_quaff(struct actor *const actor, struct item *const item)
{
    // is the item quaffable?
    const struct item_datum *const item_datum = &item_data[item->type];
    if (item_datum->type != BASE_ITEM_TYPE_POTION)
    {
        world_log(
            actor->floor,
            actor->x,
            actor->y,
            TCOD_white,
            "%s cannot quaff %s.",
            actor->name,
            item_datum->name);

        return false;
    }

    world_log(
        actor->floor,
        actor->x,
        actor->y,
        TCOD_white,
        "%s quaffs %s.",
        actor->name,
        item_datum->name);

    // cast the stored spell
    actor_cast_spell(
        actor,
        item_datum->spell_type,
        actor->x, actor->y,
        false);

    // decrement item stack
    item->stack--;

    // delete item if stack is empty
    if (item->stack <= 0)
    {
        // remove from inventory
        TCOD_list_remove(actor->items, item);

        // delete the item
        item_delete(item);
    }

    return true;
}

bool actor_read(struct actor *actor, struct item *item, int x, int y)
{
    // is the item readable?
    const struct item_datum *const item_datum = &item_data[item->type];
    if (item_datum->type != BASE_ITEM_TYPE_SCROLL &&
        item_datum->type != BASE_ITEM_TYPE_TOME)
    {
        world_log(
            actor->floor,
            actor->x,
            actor->y,
            TCOD_white,
            "%s cannot read %s.",
            actor->name,
            item_datum->name);

        return false;
    }

    world_log(
        actor->floor,
        actor->x,
        actor->y,
        TCOD_white,
        "%s reads %s.",
        actor->name,
        item_datum->name);

    if (item_datum->type == BASE_ITEM_TYPE_SCROLL)
    {
        // cast the stored spell
        actor_cast_spell(
            actor,
            item_datum->spell_type,
            x, y,
            false);

        // decrement item stack
        item->stack--;

        // delete item if stack is empty
        if (item->stack <= 0)
        {
            // remove from inventory
            TCOD_list_remove(actor->items, item);

            // delete the item
            item_delete(item);
        }
    }
    else if (item_datum->type == BASE_ITEM_TYPE_TOME)
    {
        // add spell to known spells
        TCOD_list_push(actor->known_spell_types, (void *)(size_t)item_datum->spell_type);

        // remove from inventory
        TCOD_list_remove(actor->items, item);

        // delete the item
        item_delete(item);
    }

    return true;
}

bool actor_bash(struct actor *const actor, struct object *const object)
{
    const struct object_datum *const object_datum = &object_data[object->type];

    // is the object destroyable?
    // TODO: make this a property on the object_datum?
    if (object->type == OBJECT_TYPE_STAIR_DOWN ||
        object->type == OBJECT_TYPE_STAIR_UP)
    {
        world_log(
            actor->floor,
            actor->x,
            actor->y,
            TCOD_white,
            "%s cannot destroy the %s.",
            actor->name,
            object_datum->name);

        return false;
    }

    // TODO: calculate damage properly
    // this means objects should have health
    // move damage calculation in actor_attack to a function and call it here as well

    // remove from map
    struct map *const map = &world->maps[actor->floor];
    TCOD_list_remove(map->objects, object);

    // remove from tile
    struct tile *const tile = &map->tiles[object->x][object->y];
    tile->object = NULL;

    // delete the object
    object_delete(object);

    world_log(
        actor->floor,
        actor->x,
        actor->y,
        TCOD_white,
        "%s destroys the %s.",
        actor->name,
        object_datum->name);

    return true;
}

bool actor_shoot(
    struct actor *const actor,
    const int x, const int y)
{
    // cant shoot itself!
    if (x == actor->x && y == actor->y)
    {
        return false;
    }

    // does the actor have a weapon?
    const struct item *const weapon = actor->equipment[EQUIP_SLOT_WEAPON];
    if (!weapon)
    {
        world_log(
            actor->floor,
            actor->x,
            actor->y,
            TCOD_white,
            "%s cannot shoot without a weapon!",
            actor->name);

        return false;
    }

    // is the weapon ranged?
    const struct item_datum *const weapon_datum = &item_data[weapon->type];
    const struct base_item_datum *const base_weapon_datum = &base_item_data[weapon_datum->type];
    if (!base_weapon_datum->ranged)
    {
        world_log(
            actor->floor,
            actor->x,
            actor->y,
            TCOD_white,
            "%s cannot shoot without a ranged weapon!",
            actor->name);

        return false;
    }

    // is there ammo?
    struct item *const ammunition = actor->equipment[EQUIP_SLOT_AMMUNITION];
    if (!ammunition)
    {
        world_log(
            actor->floor,
            actor->x,
            actor->y,
            TCOD_white,
            "%s has no ammunition equipped!",
            actor->name);

        return false;
    }

    // is it the right ammo?
    const struct item_datum *const ammunition_datum = &item_data[ammunition->type];
    const struct base_item_datum *const base_ammunition_datum = &base_item_data[ammunition_datum->type];
    if (base_ammunition_datum->ammunition_type != base_weapon_datum->ammunition_type)
    {
        world_log(
            actor->floor,
            actor->x,
            actor->y,
            TCOD_white,
            "%s has unsuitable ammunition equipped!",
            actor->name);

        return false;
    }

    // create a projectile
    // the projectile gets a copy of the ammunition with the stack size set to 1
    // this is so that if the actor deletes its ammunition item (e.g. if it runs out), the projectile can still use the data
    struct projectile *projectile = projectile_new(
        PROJECTILE_TYPE_ARROW,
        actor->floor,
        actor->x,
        actor->y,
        x,
        y,
        actor,
        item_new(ammunition->type, ammunition->floor, ammunition->x, ammunition->y, 1));

    // add the projectile to the map
    struct map *map = &world->maps[actor->floor];
    TCOD_list_push(map->projectiles, projectile);

    // decrement the actor's ammunition
    ammunition->stack--;

    // unequip and delete the item if out of ammo
    if (ammunition->stack <= 0)
    {
        // remove from inventory
        TCOD_list_remove(actor->items, ammunition);

        // remove from slot
        actor->equipment[EQUIP_SLOT_AMMUNITION] = NULL;

        // delete the item
        item_delete(ammunition);
    }

    return true;
}

bool actor_attack(struct actor *const actor, struct actor *const other, const struct item *const ammunition)
{
    // calculate other armor class
    const int armor_class = actor_calc_armor_class(other);

    // calculate hit
    const int attack_roll = TCOD_random_dice_roll_s(world->random, "1d20");
    const int attack_bonus = actor_calc_attack_bonus(actor);
    const int hit_challenge = attack_roll + attack_bonus;
    if (attack_roll == 1 ||
        (attack_roll != 20 && hit_challenge < armor_class))
    {
        world_log(
            actor->floor,
            actor->x,
            actor->y,
            TCOD_light_gray,
            "%s misses %s.",
            actor->name,
            other->name);

        return true;
    }

    // calculate critical hit
    int threat_roll = 0;
    int crit_challenge = 0;
    bool crit = false;

    if (attack_roll >= actor_calc_threat_range(actor))
    {
        threat_roll =
            TCOD_random_dice_roll_s(world->random, "1d20");
        crit_challenge = threat_roll + attack_bonus;
        if (crit_challenge >= armor_class)
        {
            crit = true;
        }
    }

    // calculate damage
    const int num_attack_rolls =
        crit
            ? actor_calc_critical_multiplier(actor)
            : 1;
    const int damage_bonus = actor_calc_damage_bonus(actor);
    int damage = 0;
    for (int i = 0; i < num_attack_rolls; i++)
    {
        damage +=
            TCOD_random_dice_roll_s(
                world->random,
                actor_calc_damage(actor)) +
            damage_bonus;
    }

    if (damage < 1)
    {
        damage = 1;
    }

    // TODO: when projectiles come at the player from the dark, nothing gets logged
    // it'd be nice if there were a way to do something like "someone attacks <player> for <damage>"
    if (threat_roll)
    {
        world_log(
            actor->floor,
            actor->x,
            actor->y,
            crit ? TCOD_light_red : TCOD_white,
            "%s %s %s for %d.",
            actor->name,
            crit ? "crits" : "hits",
            other->name,
            damage);
    }
    else
    {
        world_log(
            actor->floor,
            actor->x,
            actor->y,
            TCOD_white,
            "%s hits %s for %d.",
            actor->name,
            other->name,
            damage);
    }

    // deal damage
    const bool killed = actor_damage_hit_points(other, actor, damage);

    // if the other actor wasn't killed, perform any other effects
    if (!killed)
    {
        const struct item *const weapon = actor->equipment[EQUIP_SLOT_WEAPON];
        if (weapon)
        {
            if (weapon->type == ITEM_TYPE_COLD_IRON_BLADE)
            {
                other->energy -= 1.0f;

                world_log(
                    actor->floor,
                    actor->x,
                    actor->y,
                    TCOD_white,
                    "%s has been slowed!",
                    other->name);
            }

            if (weapon->type == ITEM_TYPE_SCEPTER_OF_UNITY)
            {
                other->faction = actor->faction;

                world_log(
                    actor->floor,
                    actor->x,
                    actor->y,
                    TCOD_white,
                    "%s has become friendly to %s!",
                    other->name,
                    actor->name);
            }
        }

        const struct item *const other_shield = other->equipment[EQUIP_SLOT_SHIELD];
        if (other_shield)
        {
            if (other_shield->type == ITEM_TYPE_SPIKED_SHIELD && !ammunition)
            {
                const int spike_damage = TCOD_random_dice_roll_s(world->random, "1d4");

                world_log(
                    actor->floor,
                    actor->x,
                    actor->y,
                    TCOD_white,
                    "%s's shield spike hits %s for %d.",
                    other->name,
                    actor->name,
                    spike_damage);

                actor_damage_hit_points(actor, other, spike_damage);
            }
        }
    }

    return true;
}

bool actor_cast_spell(
    struct actor *const actor,
    const enum spell_type spell_type,
    const int x, const int y,
    const bool from_memory)
{
    if (!map_is_inside(x, y))
    {
        return false;
    }

    const struct spell_datum *const spell_datum = &spell_data[spell_type];

    if (from_memory)
    {
        // does the actor know the spell?
        // this should never happen, but just in case
        // an actor should only be able to "ready" a spell they know
        if (!TCOD_list_contains(actor->known_spell_types, (void *)(size_t)spell_type))
        {
            world_log(
                actor->floor,
                actor->x,
                actor->y,
                TCOD_white,
                "%s does not know %s.",
                actor->name,
                spell_datum->name);

            return false;
        }

        // does the actor have enough mana?
        if (actor->mana_points < spell_datum->mana_cost)
        {
            world_log(
                actor->floor,
                actor->x,
                actor->y,
                TCOD_white,
                "%s does not have enough mana.",
                actor->name,
                spell_datum->name);

            return false;
        }
    }

    world_log(
        actor->floor,
        actor->x,
        actor->y,
        TCOD_purple,
        "%s casts %s.",
        actor->name,
        spell_datum->name);

    if (from_memory)
    {
        actor->mana_points -= spell_datum->mana_cost;
    }

    switch (spell_type)
    {
    case SPELL_TYPE_MINOR_HEAL:
    {
        const int health = TCOD_random_dice_roll_s(world->random, "1d4") + actor_calc_ability_modifer(actor, ABILITY_INTELLIGENCE);

        world_log(
            actor->floor,
            actor->x,
            actor->y,
            TCOD_white,
            "%s heals for %d.",
            actor->name,
            health);

        actor_restore_hit_points(actor, health);
    }
    break;
    case SPELL_TYPE_MINOR_MANA:
    {
        const int mana = TCOD_random_dice_roll_s(world->random, "1d4") + actor_calc_ability_modifer(actor, ABILITY_INTELLIGENCE);

        world_log(
            actor->floor,
            actor->x,
            actor->y,
            TCOD_white,
            "%s recovers %d mana.",
            actor->name,
            mana);

        actor_restore_mana_points(actor, mana);
    }
    break;
    case SPELL_TYPE_LIGHTNING:
    {
        struct map *const map = &world->maps[actor->floor];
        struct tile *const tile = &map->tiles[x][y];
        struct actor *const other = tile->actor;
        if (other)
        {
            const int damage = TCOD_random_dice_roll_s(world->random, "1d4") + actor_calc_ability_modifer(actor, ABILITY_INTELLIGENCE);

            world_log(
                actor->floor,
                actor->x,
                actor->y,
                TCOD_white,
                "%s zaps %s for %d.",
                actor->name,
                other->name,
                damage);

            actor_damage_hit_points(other, actor, damage);
        }
        else
        {
            world_log(
                actor->floor,
                actor->x,
                actor->y,
                TCOD_white,
                "%s cannot cast %s here.",
                actor->name,
                spell_datum->name);

            return false;
        }
    }
    break;
    case SPELL_TYPE_FIREBALL:
    {
        if (x == actor->x && y == actor->y)
        {
            return false;
        }

        // create a fireball projectile
        struct projectile *const projectile = projectile_new(
            PROJECTILE_TYPE_FIREBALL,
            actor->floor,
            actor->x,
            actor->y,
            x,
            y,
            actor,
            NULL);

        // add the projectile to the map
        struct map *const map = &world->maps[actor->floor];
        TCOD_list_push(map->projectiles, projectile);

        return false;
    }
    break;
    case NUM_SPELL_TYPES:
        break;
    }

    return true;
}

void actor_restore_hit_points(struct actor *const actor, const int health)
{
    actor->hit_points += health;
    actor->flash_color = TCOD_green;
    actor->flash_fade_coef = 1.0f;

    const int max_health = actor_calc_max_hit_points(actor);
    if (actor->hit_points > max_health)
    {
        actor->hit_points = max_health;
    }
}

void actor_restore_mana_points(struct actor *const actor, const int mana)
{
    actor->mana_points += mana;

    const int max_mana = actor_calc_max_mana_points(actor);
    if (actor->mana_points > max_mana)
    {
        actor->mana_points = max_mana;
    }
}

bool actor_damage_hit_points(struct actor *const actor, struct actor *const attacker, const int damage)
{
    actor->hit_points -= damage;
    actor->flash_color = TCOD_red;
    actor->flash_fade_coef = 1.0f;

    if (actor->hit_points <= 0)
    {
        actor->hit_points = 0;

        actor_die(actor, attacker);

        return true;
    }

    return false;
}

void actor_die(struct actor *const actor, struct actor *const killer)
{
    // can't remove the actor from the map yet, so mark it for deletion
    // this function has a high likelihoood of being called while iterating through the map's actors array,
    // so we shouldn't be removing/deleting actors while doing that
    actor->dead = true;

    world_log(
        actor->floor,
        actor->x,
        actor->y,
        TCOD_red,
        "%s dies.",
        actor->name);

    if (killer)
    {
        const int experience = actor->level * 100;
        actor_give_experience(killer, experience);
    }
}
