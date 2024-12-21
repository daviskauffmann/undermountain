#include "actor.h"

#include "ammunition_type.h"
#include "armor_proficiency.h"
#include "base_attack_bonus.h"
#include "base_item.h"
#include "class.h"
#include "color.h"
#include "equippability.h"
#include "explosion.h"
#include "light.h"
#include "list.h"
#include "magic_type.h"
#include "natural_weapon.h"
#include "object.h"
#include "projectile.h"
#include "race.h"
#include "room.h"
#include "saving_throw.h"
#include "surface.h"
#include "util.h"
#include "world.h"
#include <float.h>
#include <malloc.h>
#include <math.h>
#include <string.h>

const struct actor_metadata actor_metadata = {
    .turns_to_chase = 10,
};

struct actor *actor_new(
    const char *const name,
    const enum race race,
    const enum class class,
    const enum size size,
    const enum faction faction,
    const int level,
    const int ability_scores[NUM_ABILITIES],
    const bool special_abilities[NUM_SPECIAL_ABILITIES],
    const bool special_attacks[NUM_SPECIAL_ATTACKS],
    const bool feats[NUM_FEATS],
    const int floor,
    const int x,
    const int y)
{
    struct actor *const actor = malloc(sizeof(*actor));

    actor->name = strdup(name);

    actor->race = race;
    actor->class = class;
    actor->size = size;
    actor->faction = faction;

    actor->level = 1;
    actor->experience = actor_get_experience_for_level(level);

    actor->ability_points = 0;
    memcpy(actor->ability_scores, ability_scores, sizeof(actor->ability_scores));

    memcpy(actor->special_abilities, special_abilities, sizeof(actor->special_abilities));

    memcpy(actor->special_attacks, special_attacks, sizeof(actor->special_attacks));

    memcpy(actor->feats, feats, sizeof(actor->feats));

    actor->base_hit_points = TCOD_random_dice_new(class_database[actor->class].hit_die).nb_faces;
    actor->hit_points = actor_get_max_hit_points(actor);

    actor->gold = 0;
    for (enum equip_slot equip_slot = 0; equip_slot < NUM_EQUIP_SLOTS; equip_slot++)
    {
        actor->equipment[equip_slot] = NULL;
    }
    actor->items = list_new();

    actor->mana = actor_get_max_mana(actor);
    for (enum spell_type spell_type = 0; spell_type < NUM_SPELL_TYPES; spell_type++)
    {
        actor->spells[spell_type] = false;
    }
    actor->readied_spell = SPELL_TYPE_NONE;

    actor->floor = floor;
    actor->x = x;
    actor->y = y;

    actor->fov = NULL;

    actor->took_turn = false;
    actor->energy = actor_get_speed(actor);

    actor->current_target = NULL;
    actor->last_seen_x = -1;
    actor->last_seen_y = -1;
    actor->turns_chased = 0;

    actor->last_attacked_target = NULL;

    actor->leader = NULL;

    actor->light_type = LIGHT_TYPE_NONE;
    actor->light_fov = NULL;

    actor->flash_color = color_white;
    actor->flash_alpha = 0;

    actor->controllable = false;

    actor->dead = false;

    while (actor->level != level)
    {
        actor_level_up(actor);
    }

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

    for (enum equip_slot equip_slot = 0; equip_slot < NUM_EQUIP_SLOTS; equip_slot++)
    {
        if (actor->equipment[equip_slot] != NULL)
        {
            item_delete(actor->equipment[equip_slot]);
        }
    }

    for (size_t item_index = 0; item_index < actor->items->size; item_index++)
    {
        item_delete(list_get(actor->items, item_index));
    }
    list_delete(actor->items);

    free(actor->name);

    free(actor);
}

int actor_get_experience_for_level(const int level)
{
    return level * (level - 1) / 2 * 1000;
}

void actor_give_experience(struct actor *const actor, const int experience)
{
    actor->experience += experience;

    if (actor == world->player)
    {
        world_log(
            actor->floor,
            actor->x,
            actor->y,
            color_azure,
            "%s gains %d experience.",
            actor->name,
            experience);
    }

    while (actor->experience >= actor_get_experience_for_level(actor->level + 1))
    {
        actor_level_up(actor);

        if (actor == world->player)
        {
            world_log(
                actor->floor,
                actor->x,
                actor->y,
                color_yellow,
                "%s has gained a level!",
                actor->name);
        }
    }
}

void actor_level_up(struct actor *const actor)
{
    const float current_hit_point_percent = (float)actor->hit_points / actor_get_max_hit_points(actor);
    const float current_mana_percent = (float)actor->mana / actor_get_max_mana(actor);

    actor->level++;
    actor->ability_points++; // TODO: revisit this, may be unbalanced to grant an ability point every level
    actor->base_hit_points += TCOD_random_dice_roll_s(NULL, class_database[actor->class].hit_die);

    actor->hit_points = (int)(actor_get_max_hit_points(actor) * current_hit_point_percent);
    actor->mana = (int)(actor_get_max_mana(actor) * current_mana_percent);

    if (actor->hit_points <= 0)
    {
        actor_die(actor, NULL);
    }
    if (actor->mana <= 0)
    {
        actor->mana = 0;
    }
}

int actor_get_ability_score(const struct actor *const actor, const enum ability ability)
{
    return actor->ability_scores[ability] + race_database[actor->race].ability_adjustments[ability];
}

int actor_get_ability_modifer(const struct actor *const actor, const enum ability ability)
{
    int modifier = (int)floorf((actor_get_ability_score(actor, ability) - 10) / 2.0f);

    // apply maximum dexterity bonus from armor
    if (ability == ABILITY_DEXTERITY)
    {
        const struct item *const armor = actor->equipment[EQUIP_SLOT_ARMOR];

        if (armor)
        {
            const int max_dexterity_bonus = base_item_database[item_database[armor->type].type].max_dexterity_bonus;

            if (max_dexterity_bonus > 0 &&
                modifier > max_dexterity_bonus)
            {
                modifier = max_dexterity_bonus;
            }
        }
    }

    return modifier;
}

void actor_spend_ability_point(struct actor *const actor, const enum ability ability)
{
    const float current_hit_point_percent = (float)actor->hit_points / actor_get_max_hit_points(actor);
    const float current_mana_percent = (float)actor->mana / actor_get_max_mana(actor);

    actor->ability_scores[ability]++;
    actor->ability_points--;

    actor->hit_points = (int)(actor_get_max_hit_points(actor) * current_hit_point_percent);
    actor->mana = (int)(actor_get_max_mana(actor) * current_mana_percent);

    if (actor->hit_points <= 0)
    {
        actor_die(actor, NULL);
    }
    if (actor->mana <= 0)
    {
        actor->mana = 0;
    }
}

struct actor_special_abilities actor_get_special_abilities(const struct actor *const actor)
{
    struct actor_special_abilities actor_special_abilities = {
        .has = {false},
    };

    for (enum special_ability special_ability = 0; special_ability < NUM_SPECIAL_ABILITIES; special_ability++)
    {
        if (race_database[actor->race].special_abilities[special_ability])
        {
            actor_special_abilities.has[special_ability] = true;
        }

        if (actor->special_abilities[special_ability])
        {
            actor_special_abilities.has[special_ability] = true;
        }
    }

    return actor_special_abilities;
}

struct actor_special_attacks actor_get_special_attacks(const struct actor *const actor)
{
    struct actor_special_attacks actor_special_attacks = {
        .has = {false},
    };

    for (enum special_attack special_attack = 0; special_attack < NUM_SPECIAL_ATTACKS; special_attack++)
    {
        if (actor->special_attacks[special_attack])
        {
            actor_special_attacks.has[special_attack] = true;
        }
    }

    return actor_special_attacks;
}

struct actor_feats actor_get_feats(const struct actor *actor)
{
    struct actor_feats actor_feats = {
        .has = {false},
    };

    for (enum feat feat = 0; feat < NUM_FEATS; feat++)
    {
        if (race_database[actor->race].feats[feat])
        {
            actor_feats.has[feat] = true;
        }

        const int level = class_database[actor->class].feat_progression[feat];

        if (level > 0 && level <= actor->level)
        {
            actor_feats.has[feat] = true;
        }

        if (actor->feats[feat])
        {
            actor_feats.has[feat] = true;
        }

        // TODO: feats from equipment
    }

    return actor_feats;
}

bool actor_has_prerequisites_for_feat(const struct actor *actor, const enum feat feat)
{
    const struct actor_feats actor_feats = actor_get_feats(actor);

    if (actor_feats.has[feat])
    {
        // TODO: if getting the feat from equipment, the actor should still be able to learn it
        return false;
    }

    const struct feat_prerequisites *const prerequisites = &feat_database[feat].prerequisites;

    if (prerequisites->race != RACE_NONE &&
        prerequisites->race != actor->race)
    {
        return false;
    }

    if (prerequisites->class != CLASS_NONE &&
        prerequisites->class != actor->class)
    {
        return false;
    }

    if (prerequisites->level > actor->level)
    {
        return false;
    }

    if (feat_database[feat].prerequisites.base_attack_bonus > actor_get_base_attack_bonus(actor))
    {
        return false;
    }

    for (enum ability ability = ABILITY_NONE + 1; ability < NUM_ABILITIES; ability++)
    {
        if (prerequisites->ability_scores[ability] > 0 &&
            actor->ability_scores[ability] < prerequisites->ability_scores[ability])
        {
            return false;
        }
    }

    for (enum feat _feat = FEAT_NONE + 1; _feat < NUM_FEATS; _feat++)
    {
        if (prerequisites->feats[_feat] &&
            !actor_feats.has[_feat])
        {
            return false;
        }
    }

    return true;
}

struct actor_spells actor_get_spells(const struct actor *const actor)
{
    struct actor_spells spells = {
        .has = {false},
    };

    for (enum spell_type spell_type = SPELL_TYPE_NONE + 1; spell_type < NUM_SPELL_TYPES; spell_type++)
    {
        const struct class_data *class_data = &class_database[actor->class];
        const int level = class_data->spell_progression[spell_type];

        if (level > 0 &&
            level <= actor->level &&
            level <= 10 + actor->ability_scores[class_data->spellcasting_ability])
        {
            spells.has[spell_type] = true;
        }

        if (actor->spells[spell_type])
        {
            spells.has[spell_type] = true;
        }
    }

    return spells;
}

int actor_get_max_hit_points(const struct actor *const actor)
{
    const int base_hit_points = actor->base_hit_points;
    const int constitution_modifer = actor_get_ability_modifer(actor, ABILITY_CONSTITUTION);

    int max_hit_points = base_hit_points + (constitution_modifer * actor->level);

    const struct actor_feats actor_feats = actor_get_feats(actor);

    if (actor_feats.has[FEAT_TOUGHNESS])
    {
        max_hit_points += actor->level;
    }

    return max_hit_points;
}

void actor_restore_hit_points(struct actor *const actor, const int health)
{
    actor->hit_points += health;
    actor->flash_color = color_green;
    actor->flash_alpha = 1;

    const int max_hit_points = actor_get_max_hit_points(actor);

    if (actor->hit_points > max_hit_points)
    {
        actor->hit_points = max_hit_points;
    }
}

bool actor_damage_hit_points(struct actor *const actor, struct actor *const attacker, int damage)
{
    const struct item *const armor = actor->equipment[EQUIP_SLOT_ARMOR];

    if (armor && armor->type == ITEM_TYPE_ADAMANTINE_BREASTPLATE)
    {
        damage -= 2;

        if (damage < 0)
        {
            return true;
        }
    }

    actor->hit_points -= damage;
    actor->flash_color = color_red;
    actor->flash_alpha = 1;

    if (actor->hit_points <= 0)
    {
        actor->hit_points = 0;

        actor_die(actor, attacker);

        return true;
    }

    return false;
}

int actor_get_max_mana(const struct actor *const actor)
{
    const enum ability spellcasting_ability = class_database[actor->class].spellcasting_ability;

    if (spellcasting_ability == ABILITY_NONE)
    {
        return 0;
    }

    return actor->level * actor_get_ability_modifer(actor, spellcasting_ability);
}

void actor_restore_mana(struct actor *const actor, const int mana)
{
    actor->mana += mana;

    const int max_mana = actor_get_max_mana(actor);
    if (actor->mana > max_mana)
    {
        actor->mana = max_mana;
    }
}

float actor_get_arcane_spell_failure(const struct actor *const actor)
{
    const struct actor_feats actor_feats = actor_get_feats(actor);

    if (actor_feats.has[FEAT_STILL_SPELL])
    {
        return 0;
    }

    float arcane_spell_failure = 0;

    for (enum equip_slot equip_slot = EQUIP_SLOT_NONE + 1; equip_slot < NUM_EQUIP_SLOTS; equip_slot++)
    {
        const struct item *const item = actor->equipment[equip_slot];

        if (item)
        {
            arcane_spell_failure += base_item_database[item_database[item->type].type].arcane_spell_failure;
        }
    }

    return arcane_spell_failure;
}

int actor_get_spell_mana_cost(const struct actor *const actor, const enum spell_type spell_type)
{
    int mana_cost = spell_database[spell_type].level;

    const struct actor_feats actor_feats = actor_get_feats(actor);

    if (actor_feats.has[FEAT_STILL_SPELL])
    {
        mana_cost++;
    }

    return mana_cost;
}

int actor_get_saving_throw(const struct actor *const actor, const enum saving_throw saving_throw)
{
    const int base_save = actor->level / 2 + 2;
    const int ability_modifer = actor_get_ability_modifer(actor, saving_throw_database[saving_throw].ability);

    int value = base_save + ability_modifer;

    const struct actor_feats actor_feats = actor_get_feats(actor);

    if (saving_throw == SAVING_THROW_FORTITUDE && actor_feats.has[FEAT_GREAT_FORTITUDE])
    {
        value += 2;
    }

    if (saving_throw == SAVING_THROW_REFLEX && actor_feats.has[FEAT_LIGHTNING_REFLEXES])
    {
        value += 2;
    }

    if (saving_throw == SAVING_THROW_WILL && actor_feats.has[FEAT_IRON_WILL])
    {
        value += 2;
    }

    return value;
}

int actor_get_armor_class(const struct actor *const actor)
{
    int armor_class =
        10 +
        actor_get_ability_modifer(actor, ABILITY_DEXTERITY) +
        class_database[actor->class].natural_armor_bonus +
        size_database[actor->size].modifier;

    for (enum equip_slot equip_slot = EQUIP_SLOT_NONE + 1; equip_slot < NUM_EQUIP_SLOTS; equip_slot++)
    {
        const struct item *const item = actor->equipment[equip_slot];

        if (item)
        {
            const struct item_data *const item_data = &item_database[item->type];

            armor_class += base_item_database[item_data->type].armor_class;
            armor_class += item_data->enhancement_bonus;
        }
    }

    const struct actor_feats actor_feats = actor_get_feats(actor);

    if (actor_feats.has[FEAT_DODGE])
    {
        armor_class += 1;
    }

    return armor_class;
}

int actor_get_attacks_per_round(const struct actor *const actor)
{
    // actors using crossbows without the rapid reload feat can only make one attack per round
    const struct item *const weapon = actor->equipment[EQUIP_SLOT_WEAPON];

    if (weapon)
    {
        const struct item_data *const weapon_data = &item_database[weapon->type];
        const struct actor_feats actor_feats = actor_get_feats(actor);

        if (base_item_database[weapon_data->type].ranged &&
            (weapon_data->type == BASE_ITEM_TYPE_LIGHT_CROSSBOW ||
             weapon_data->type == BASE_ITEM_TYPE_HEAVY_CROSSBOW) &&
            !actor_feats.has[FEAT_RAPID_RELOAD])
        {
            return 1;
        }
    }

    const int base_attack_bonus = actor_get_base_attack_bonus(actor);

    int attacks_per_round = 1;

    if (base_attack_bonus > 5)
    {
        attacks_per_round++;
    }
    if (base_attack_bonus > 10)
    {
        attacks_per_round++;
    }
    if (base_attack_bonus > 15)
    {
        attacks_per_round++;
    }

    // TODO: monk using unarmed or kama
    // return (base_attack_bonus - 3) / 3;

    // TODO: dual wielding? off hand weapon gives extra attacks at BAB -10 / 5

    // TODO: rapid shot feat, gives extra attack per round if ranged (not crossbow)
    // need to implement this in the attack code as well, since all attacks in round suffer -2 penalty

    return attacks_per_round;
}

int actor_get_successive_attack_penalty(const struct actor *const actor)
{
    const struct actor_feats actor_feats = actor_get_feats(actor);

    if (actor_feats.has[FEAT_MULTIATTACK])
    {
        return 2;
    }

    return 5;
}

int actor_get_base_attack_bonus(const struct actor *const actor)
{
    return (int)floorf(actor->level * base_attack_bonus_database[class_database[actor->class].base_attack_bonus_type].multiplier);
}

int actor_get_attack_bonus(const struct actor *const actor)
{
    int attack_bonus = actor_get_base_attack_bonus(actor) + size_database[actor->size].modifier;

    const struct item *const weapon = actor->equipment[EQUIP_SLOT_WEAPON];
    const struct actor_feats actor_feats = actor_get_feats(actor);

    if (weapon)
    {
        const struct item_data *const weapon_data = &item_database[weapon->type];

        attack_bonus += weapon_data->enhancement_bonus;

        const struct base_item_data *const base_weapon_data = &base_item_database[weapon_data->type];

        // ranged weapons use dexterity modifier instead of strength modifier
        if (base_weapon_data->ranged)
        {
            attack_bonus += actor_get_ability_modifer(actor, ABILITY_DEXTERITY);

            const struct item *const ammunition = actor->equipment[EQUIP_SLOT_AMMUNITION];

            if (ammunition)
            {
                attack_bonus += item_database[ammunition->type].enhancement_bonus;
            }
        }
        else
        {
            const enum equippability equippability = actor_get_item_equippability(actor, weapon);

            // if the actor has the weapon finesse feat, the weapon is light, and is not two-handed, then use dexterity modifier instead of strength modifier
            if (actor_feats.has[FEAT_WEAPON_FINESSE] &&
                base_weapon_data->light &&
                equippability != EQUIPPABILITY_BARELY)
            {
                attack_bonus += actor_get_ability_modifer(actor, ABILITY_DEXTERITY);
            }
            else
            {
                attack_bonus += actor_get_ability_modifer(actor, ABILITY_STRENGTH);
            }
        }

        if (actor_feats.has[FEAT_WEAPON_FOCUS])
        {
            attack_bonus += 1;
        }
    }
    else
    {
        // unarmed attacks are always considered light, so if the actor has the weapon finesse feat, then use dexterity modifier instead of strength modifier
        if (actor_feats.has[FEAT_WEAPON_FINESSE])
        {
            attack_bonus += actor_get_ability_modifer(actor, ABILITY_DEXTERITY);
        }
        else
        {
            attack_bonus += actor_get_ability_modifer(actor, ABILITY_STRENGTH);
        }
    }

    if (actor_feats.has[FEAT_POWER_ATTACK])
    {
        attack_bonus -= 5;
    }

    return attack_bonus;
}

int actor_get_ranged_attack_penalty(const struct actor *const actor, const struct actor *const other)
{
    const struct item *const weapon = actor->equipment[EQUIP_SLOT_WEAPON];

    if (weapon)
    {
        // ranged weapons fired at close range suffer an automatic -4 attack penalty, unless the actor has the point blank shot feat
        const struct actor_feats actor_feats = actor_get_feats(actor);

        if (base_item_database[item_database[weapon->type].type].ranged &&
            distance_between(actor->x, actor->y, other->x, other->y) < 2 &&
            !actor_feats.has[FEAT_POINT_BLANK_SHOT])
        {
            return 4;
        }
    }

    return 0;
}

int actor_get_threat_range(const struct actor *const actor)
{
    const struct item *const weapon = actor->equipment[EQUIP_SLOT_WEAPON];

    if (weapon)
    {
        const struct item_data weapon_data = item_database[weapon->type];
        const struct base_item_data base_weapon_data = base_item_database[weapon_data.type];

        return base_weapon_data.threat_range;
    }
    const enum natural_weapon_type natural_weapon_type = class_database[actor->class].natural_weapon_type;

    return natural_weapon_database[natural_weapon_type].threat_range;
}

int actor_get_critical_multiplier(const struct actor *const actor)
{
    const struct item *const weapon = actor->equipment[EQUIP_SLOT_WEAPON];

    if (weapon)
    {
        const struct item_data weapon_data = item_database[weapon->type];
        const struct base_item_data base_weapon_data = base_item_database[weapon_data.type];

        return base_weapon_data.critical_multiplier;
    }

    const enum natural_weapon_type natural_weapon_type = class_database[actor->class].natural_weapon_type;

    return natural_weapon_database[natural_weapon_type].critical_multiplier;
}

int actor_get_damage_bonus(const struct actor *const actor)
{
    int damage_bonus = 0;

    const struct item *const weapon = actor->equipment[EQUIP_SLOT_WEAPON];
    const struct actor_feats actor_feats = actor_get_feats(actor);

    if (weapon)
    {
        const struct item_data weapon_data = item_database[weapon->type];

        damage_bonus += weapon_data.enhancement_bonus;

        const struct base_item_data base_weapon_data = base_item_database[weapon_data.type];

        if (base_weapon_data.ranged)
        {
            // ranged weapons do not get strength modifier to damage, except for slings
            // though, they do receive strength penalties, except for composite bows
            const int strength_modifier = actor_get_ability_modifer(actor, ABILITY_STRENGTH);

            if (weapon_data.type == BASE_ITEM_TYPE_SLING)
            {
                damage_bonus += strength_modifier;
            }
            else if (strength_modifier < 0 && weapon_data.type != BASE_ITEM_TYPE_COMPOSITE_BOW)
            {
                damage_bonus += strength_modifier;
            }

            const struct item *const ammunition = actor->equipment[EQUIP_SLOT_AMMUNITION];

            if (ammunition)
            {
                const struct item_data ammunition_data = item_database[ammunition->type];

                damage_bonus += ammunition_data.enhancement_bonus;
            }
        }
        else
        {
            const enum equippability equippability = actor_get_item_equippability(actor, weapon);

            // two-handing a weapon adds 1.5x damage bonus
            if (equippability == EQUIPPABILITY_BARELY)
            {
                damage_bonus += (int)(actor_get_ability_modifer(actor, ABILITY_STRENGTH) * 1.5f);
            }
            else
            {
                damage_bonus += actor_get_ability_modifer(actor, ABILITY_STRENGTH);
            }

            // power attack only works with melee attacks
            if (actor_feats.has[FEAT_POWER_ATTACK])
            {
                damage_bonus += 5;
            }
        }
    }
    else
    {
        damage_bonus += actor_get_ability_modifer(actor, ABILITY_STRENGTH);

        // power attack only works with melee attacks
        if (actor_feats.has[FEAT_POWER_ATTACK])
        {
            damage_bonus += 5;
        }
    }

    return damage_bonus;
}

const char *actor_get_damage(const struct actor *const actor)
{
    const struct item *const weapon = actor->equipment[EQUIP_SLOT_WEAPON];

    if (weapon)
    {
        return base_item_database[item_database[weapon->type].type].damage;
    }

    return natural_weapon_database[class_database[actor->class].natural_weapon_type].damage;
}

bool actor_melee_touch_attack(struct actor *const actor, struct actor *const other)
{
    const int attack_roll = TCOD_random_dice_roll_s(NULL, "1d20");

    if (attack_roll == 1)
    {
        return false;
    }

    if (attack_roll == 20)
    {
        return true;
    }

    const int hit_challenge =
        attack_roll +
        actor_get_base_attack_bonus(actor) +
        actor_get_ability_modifer(actor, ABILITY_STRENGTH);
    const int armor_class = 10 + actor_get_ability_modifer(other, ABILITY_DEXTERITY);

    if (hit_challenge < armor_class)
    {
        return false;
    }

    return true;
}

bool actor_ranged_touch_attack(struct actor *const actor, struct actor *const other)
{
    const int attack_roll = TCOD_random_dice_roll_s(NULL, "1d20");

    if (attack_roll == 1)
    {
        return false;
    }

    if (attack_roll == 20)
    {
        return true;
    }

    const int hit_challenge =
        attack_roll +
        actor_get_base_attack_bonus(actor) +
        actor_get_ability_modifer(actor, ABILITY_DEXTERITY);
    const int armor_class = 10 + actor_get_ability_modifer(other, ABILITY_DEXTERITY);

    if (hit_challenge < armor_class)
    {
        return false;
    }

    return true;
}

enum equippability actor_get_item_equippability(const struct actor *const actor, const struct item *const item)
{
    const int difference = actor->size - base_item_database[item_database[item->type].type].size;

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

float actor_get_max_carry_weight(const struct actor *actor)
{
    return 100 + actor_get_ability_modifer(actor, ABILITY_STRENGTH) * 10.0f;
}

float actor_get_carry_weight(const struct actor *actor)
{
    float weight = 0.0f;

    for (size_t item_index = 0; item_index < actor->items->size; item_index++)
    {
        const struct item *const item = list_get(actor->items, item_index);

        weight += base_item_database[item_database[item->type].type].weight;
    }

    for (enum equip_slot equip_slot = 1; equip_slot < NUM_EQUIP_SLOTS; equip_slot++)
    {
        const struct item *const item = actor->equipment[equip_slot];

        if (item)
        {
            weight += base_item_database[item_database[item->type].type].weight;
        }
    }

    return weight;
}

float actor_get_speed(const struct actor *actor)
{
    return size_database[actor->size].speed * (actor_get_carry_weight(actor) > actor_get_max_carry_weight(actor) ? 0.5f : 1);
}

void actor_update_light(struct actor *const actor)
{
    if (actor->light_fov)
    {
        TCOD_map_delete(actor->light_fov);
        actor->light_fov = NULL;
    }

    const struct map *map = &world->maps[actor->floor];
    const int radius = light_database[actor->light_type].radius;

    if (radius >= 0)
    {
        actor->light_fov = map_to_fov_map(
            map,
            actor->x,
            actor->y,
            radius);
    }
    else if (actor->light_type == LIGHT_TYPE_PLAYER)
    {
        const int sight_radius = actor_get_sight_radius(actor);

        actor->light_fov = map_to_fov_map(
            map,
            actor->x,
            actor->y,
            sight_radius);
    }
}

void actor_update_fade(struct actor *const actor, const float delta_time)
{
    if (actor->flash_alpha > 0)
    {
        // TODO: slower/faster fade depending on circumstances
        // TODO: different fade functions such as sin()
        actor->flash_alpha -= 4 * delta_time;
    }
    else
    {
        actor->flash_alpha = 0;
    }
}

int actor_get_sight_radius(const struct actor *actor)
{
    const struct actor_special_abilities actor_special_abilities = actor_get_special_abilities(actor);

    if (actor_special_abilities.has[SPECIAL_ABILITY_DARKVISION])
    {
        return 4;
    }

    if (actor_special_abilities.has[SPECIAL_ABILITY_LOW_LIGHT_VISION])
    {
        return 3;
    }

    return 2;
}

void actor_update_fov(struct actor *const actor)
{
    if (actor->fov)
    {
        TCOD_map_delete(actor->fov);
    }

    struct map *const map = &world->maps[actor->floor];

    actor->fov = map_to_fov_map(map, actor->x, actor->y, actor_get_sight_radius(actor));

    TCOD_Map *los_map = map_to_fov_map(map, actor->x, actor->y, 0);

    for (int x = 0; x < MAP_WIDTH; x++)
    {
        for (int y = 0; y < MAP_HEIGHT; y++)
        {
            bool in_fov = false;

            if (!TCOD_map_is_in_fov(actor->fov, x, y) &&
                TCOD_map_is_in_fov(los_map, x, y))
            {
                for (size_t object_index = 0; object_index < map->objects->size; object_index++)
                {
                    const struct object *const object = list_get(map->objects, object_index);

                    if (object->light_fov &&
                        TCOD_map_is_in_fov(object->light_fov, x, y))
                    {
                        in_fov = true;

                        goto next_tile;
                    }
                }

                for (size_t actor_index = 0; actor_index < map->actors->size; actor_index++)
                {
                    const struct actor *const other = list_get(map->actors, actor_index);

                    if (other->light_type != LIGHT_TYPE_PLAYER &&
                        other->light_fov &&
                        TCOD_map_is_in_fov(other->light_fov, x, y))
                    {
                        in_fov = true;

                        goto next_tile;
                    }
                }

                for (size_t projectile_index = 0; projectile_index < map->projectiles->size; projectile_index++)
                {
                    const struct projectile *const projectile = list_get(map->projectiles, projectile_index);

                    if (projectile->light_fov &&
                        TCOD_map_is_in_fov(projectile->light_fov, x, y))
                    {
                        in_fov = true;

                        goto next_tile;
                    }
                }

                for (size_t explosion_index = 0; explosion_index < map->explosions->size; explosion_index++)
                {
                    const struct explosion *const explosion = list_get(map->explosions, explosion_index);

                    if (explosion->fov &&
                        TCOD_map_is_in_fov(explosion->fov, x, y))
                    {
                        in_fov = true;

                        goto next_tile;
                    }
                }

                for (size_t surface_index = 0; surface_index < map->surfaces->size; surface_index++)
                {
                    const struct surface *const surface = list_get(map->surfaces, surface_index);

                    if (surface->light_fov &&
                        TCOD_map_is_in_fov(surface->light_fov, x, y))
                    {
                        in_fov = true;

                        goto next_tile;
                    }
                }
            }

        next_tile:;
            if (in_fov)
            {
                TCOD_map_set_in_fov(actor->fov, x, y, true);
            }
        }
    }

    if (actor == world->player)
    {
        for (int x = 0; x < MAP_WIDTH; x++)
        {
            for (int y = 0; y < MAP_HEIGHT; y++)
            {
                if (TCOD_map_is_in_fov(actor->fov, x, y))
                {
                    map->tiles[x][y].explored = true;
                }
            }
        }
    }

    TCOD_map_delete(los_map);
}

bool actor_can_take_turn(const struct actor *const actor)
{
    return !world->doomed && actor->energy >= 1 && !actor->dead;
}

bool actor_is_enemy(const struct actor *const actor, const struct actor *const other)
{
    return TCOD_map_is_in_fov(actor->fov, other->x, other->y) &&
           actor->faction != other->faction;
}

bool actor_is_enemy_nearby(const struct actor *const actor)
{
    const struct map *const map = &world->maps[actor->floor];

    for (size_t actor_index = 0; actor_index < map->actors->size; actor_index++)
    {
        struct actor *const other = list_get(map->actors, actor_index);

        if (actor_is_enemy(actor, other))
        {
            return true;
        }
    }

    return false;
}

struct actor *actor_find_nearest_enemy(const struct actor *const actor)
{
    const struct map *const map = &world->maps[actor->floor];

    struct actor *nearest_enemy = NULL;
    float min_distance = FLT_MAX;

    for (size_t actor_index = 0; actor_index < map->actors->size; actor_index++)
    {
        struct actor *const other = list_get(map->actors, actor_index);

        if (actor_is_enemy(actor, other))
        {
            const float distance = distance_between_sq(
                actor->x, actor->y,
                other->x, other->y);

            if (distance < min_distance)
            {
                nearest_enemy = other;
                min_distance = distance;
            }
        }
    }

    return nearest_enemy;
}

bool actor_is_proficient(const struct actor *const actor, const struct item *const item)
{
    const struct base_item_data base_item_data = base_item_database[item_database[item->type].type];
    const struct actor_feats actor_feats = actor_get_feats(actor);

    if (base_item_data.equip_slot == EQUIP_SLOT_WEAPON)
    {
        bool is_weapon_proficient = false;

        for (enum weapon_proficiency weapon_proficiency = WEAPON_PROFICIENCY_NONE + 1; weapon_proficiency < NUM_WEAPON_PROFICIENCIES; weapon_proficiency++)
        {
            if (base_item_data.weapon_proficiencies[weapon_proficiency])
            {
                if (weapon_proficiency == WEAPON_PROFICIENCY_ELF && actor_feats.has[FEAT_WEAPON_PROFICIENCY_ELF])
                {
                    is_weapon_proficient = true;

                    break;
                }
                else if (weapon_proficiency == WEAPON_PROFICIENCY_EXOTIC && actor_feats.has[FEAT_WEAPON_PROFICIENCY_EXOTIC])
                {
                    is_weapon_proficient = true;

                    break;
                }
                else if (weapon_proficiency == WEAPON_PROFICIENCY_MARTIAL && actor_feats.has[FEAT_WEAPON_PROFICIENCY_MARTIAL])
                {
                    is_weapon_proficient = true;

                    break;
                }
                else if (weapon_proficiency == WEAPON_PROFICIENCY_ROGUE && actor_feats.has[FEAT_WEAPON_PROFICIENCY_ROGUE])
                {
                    is_weapon_proficient = true;

                    break;
                }
                else if (weapon_proficiency == WEAPON_PROFICIENCY_SIMPLE && actor_feats.has[FEAT_WEAPON_PROFICIENCY_SIMPLE])
                {
                    is_weapon_proficient = true;

                    break;
                }
                else if (weapon_proficiency == WEAPON_PROFICIENCY_WIZARD && actor_feats.has[FEAT_WEAPON_PROFICIENCY_WIZARD])
                {
                    is_weapon_proficient = true;

                    break;
                }
            }
        }

        return is_weapon_proficient;
    }
    else if (base_item_data.equip_slot == EQUIP_SLOT_ARMOR ||
             base_item_data.equip_slot == EQUIP_SLOT_SHIELD)
    {
        if (base_item_data.armor_proficiency != ARMOR_PROFICIENCY_NONE)
        {
            if (base_item_data.armor_proficiency == ARMOR_PROFICIENCY_HEAVY && actor_feats.has[FEAT_ARMOR_PROFICIENCY_HEAVY])
            {
                return true;
            }
            else if (base_item_data.armor_proficiency == ARMOR_PROFICIENCY_LIGHT && actor_feats.has[FEAT_ARMOR_PROFICIENCY_LIGHT])
            {
                return true;
            }
            else if (base_item_data.armor_proficiency == ARMOR_PROFICIENCY_MEDIUM && actor_feats.has[FEAT_ARMOR_PROFICIENCY_MEDIUM])
            {
                return true;
            }
            else if (base_item_data.armor_proficiency == ARMOR_PROFICIENCY_SHIELD && actor_feats.has[FEAT_SHIELD_PROFICIENCY])
            {
                return true;
            }
            else
            {
                return false;
            }
        }
    }

    return true;
}

struct item *actor_find_melee_weapon(const struct actor *const actor)
{
    for (size_t item_index = 0; item_index < actor->items->size; item_index++)
    {
        struct item *const item = list_get(actor->items, item_index);
        const struct base_item_data base_item_data = base_item_database[item_database[item->type].type];

        if (base_item_data.equip_slot == EQUIP_SLOT_WEAPON &&
            !base_item_data.ranged &&
            actor_is_proficient(actor, item))
        {
            return item;
        }
    }

    return NULL;
}

bool actor_has_ranged_weapon(const struct actor *actor)
{
    const struct item *const weapon = actor->equipment[EQUIP_SLOT_WEAPON];

    if (!weapon)
    {
        return false;
    }

    if (!base_item_database[item_database[weapon->type].type].ranged)
    {
        return false;
    }

    return true;
}

struct item *actor_find_ranged_weapon(const struct actor *actor)
{
    for (size_t item_index = 0; item_index < actor->items->size; item_index++)
    {
        struct item *const item = list_get(actor->items, item_index);
        const struct base_item_data base_item_data = base_item_database[item_database[item->type].type];

        if (base_item_data.equip_slot == EQUIP_SLOT_WEAPON &&
            base_item_data.ranged &&
            actor_is_proficient(actor, item))
        {
            return item;
        }
    }

    return NULL;
}

bool actor_has_ammunition(const struct actor *const actor, const struct item *const weapon)
{
    const struct base_item_data base_weapon_data = base_item_database[item_database[weapon->type].type];

    if (base_weapon_data.equip_slot != EQUIP_SLOT_WEAPON)
    {
        return false;
    }

    if (!base_weapon_data.ranged)
    {
        return false;
    }

    if (base_weapon_data.ammunition_type == AMMUNITION_TYPE_NONE)
    {
        return true;
    }

    const struct item *const ammunition = actor->equipment[EQUIP_SLOT_AMMUNITION];

    if (!ammunition)
    {
        return false;
    }

    const struct base_item_data base_ammunition_data = base_item_database[item_database[ammunition->type].type];

    if (base_ammunition_data.ammunition_type != base_weapon_data.ammunition_type)
    {
        return false;
    }

    return true;
}

struct item *actor_find_ammunition(const struct actor *const actor, const struct item *const weapon)
{
    const struct base_item_data base_weapon_data = base_item_database[item_database[weapon->type].type];

    if (base_weapon_data.equip_slot != EQUIP_SLOT_WEAPON)
    {
        return NULL;
    }

    if (!base_weapon_data.ranged)
    {
        return NULL;
    }

    if (base_weapon_data.ammunition_type == AMMUNITION_TYPE_NONE)
    {
        return NULL;
    }

    for (size_t item_index = 0; item_index < actor->items->size; item_index++)
    {
        struct item *const item = list_get(actor->items, item_index);
        const struct base_item_data base_item_data = base_item_database[item_database[item->type].type];

        if (base_item_data.equip_slot == EQUIP_SLOT_AMMUNITION &&
            base_item_data.ammunition_type == base_weapon_data.ammunition_type &&
            actor_is_proficient(actor, item))
        {
            return item;
        }
    }

    return NULL;
}

bool actor_can_make_attack_of_opportunity(const struct actor *const actor, const struct actor *const other)
{
    return actor_is_enemy(actor, other) &&
           !actor_has_ranged_weapon(actor) &&
           distance_between(actor->x, actor->y, other->x, other->y) < 2;
}

void actor_provoke_attack_of_opportunity(struct actor *actor, struct list **const damages)
{
    const struct map *const map = &world->maps[actor->floor];

    for (size_t actor_index = 0; actor_index < map->actors->size; actor_index++)
    {
        struct actor *const other = list_get(map->actors, actor_index);

        if (actor_can_make_attack_of_opportunity(other, actor))
        {
            if (damages)
            {
                int damage = 0;

                actor_attack(other, actor, true, &damage);

                if (damage > 0)
                {
                    list_add(*damages, (void *)(intptr_t)damage);
                }
            }
            else
            {
                actor_attack(other, actor, true, NULL);
            }

            if (actor->dead)
            {
                break;
            }
        }
    }
}

bool actor_ai(struct actor *const actor)
{
    const struct map *const map = &world->maps[actor->floor];

    if (actor->hit_points < actor_get_max_hit_points(actor) / 2)
    {
        // TODO: look in inventory for potion

        // TODO: cast a healing spell

        // look for fountains to heal
        const struct object *nearest_fountain = NULL;
        float min_distance = FLT_MAX;

        for (size_t object_index = 0; object_index < map->objects->size; object_index++)
        {
            const struct object *const object = list_get(map->objects, object_index);

            if (TCOD_map_is_in_fov(actor->fov, object->x, object->y) &&
                object->type == OBJECT_TYPE_FOUNTAIN)
            {
                const float distance = distance_between_sq(
                    actor->x, actor->y,
                    object->x, object->y);

                if (distance < min_distance)
                {
                    nearest_fountain = object;
                    min_distance = distance;
                }
            }
        }

        if (nearest_fountain)
        {
            if (distance_between(
                    actor->x, actor->y,
                    nearest_fountain->x, nearest_fountain->y) < 2)
            {
                if (actor_drink(actor, nearest_fountain->x, nearest_fountain->y))
                {
                    return true;
                }
            }
            else
            {
                if (actor_path_towards(actor, nearest_fountain->x, nearest_fountain->y))
                {
                    return true;
                }
            }
        }
    }

    // TODO: look for injured allies to heal

    // look for hostile targets
    struct actor *const nearest_enemy = actor_find_nearest_enemy(actor);

    if (nearest_enemy)
    {
        // target spotted, so remember the location in case the actor loses them
        actor->current_target = nearest_enemy;
        actor->last_seen_x = nearest_enemy->x;
        actor->last_seen_y = nearest_enemy->y;
        actor->turns_chased = 0;

        const struct item *const weapon = actor->equipment[EQUIP_SLOT_WEAPON];
        const struct actor_feats actor_feats = actor_get_feats(actor);

        // is the actor in melee range?
        if (distance_between(
                actor->x, actor->y,
                nearest_enemy->x, nearest_enemy->y) < 2)
        {
            // if no melee weapon, look for a melee weapon to equip
            if (!weapon)
            {
                struct item *const melee_weapon = actor_find_melee_weapon(actor);

                if (melee_weapon)
                {
                    if (actor_equip(actor, melee_weapon))
                    {
                        return true;
                    }
                }
            }

            // if carrying a ranged weapon, look for other options before shooting
            if (actor_has_ranged_weapon(actor))
            {
                // look for a melee weapon to equip
                if (!actor_feats.has[FEAT_POINT_BLANK_SHOT])
                {
                    struct item *const melee_weapon = actor_find_melee_weapon(actor);

                    if (melee_weapon)
                    {
                        if (actor_equip(actor, melee_weapon))
                        {
                            return true;
                        }
                    }
                }

                // did not find melee weapon or has point blank shot feat, so check ammo
                if (!actor_has_ammunition(actor, weapon))
                {
                    // look for suitable ammo in inventory and equip it
                    struct item *const ammunition = actor_find_ammunition(actor, weapon);

                    if (ammunition)
                    {
                        if (actor_equip(actor, ammunition))
                        {
                            return true;
                        }
                    }

                    // we didn't find ammo, so unequip the weapon
                    if (actor_unequip(actor, EQUIP_SLOT_WEAPON))
                    {
                        return true;
                    }
                }

                // no other choice, take the shot in melee range
                if (actor_shoot(actor, nearest_enemy->x, nearest_enemy->y))
                {
                    return true;
                }
            }

            // enemy is in melee range, so melee attack
            if (actor_attack(actor, nearest_enemy, false, NULL))
            {
                return true;
            }
        }
        else
        {
            // is the actor carrying a ranged weapon?
            if (actor_has_ranged_weapon(actor))
            {
                // make sure the actor has ammo
                if (!actor_has_ammunition(actor, weapon))
                {
                    // look for suitable ammo in inventory and equip it
                    struct item *const ammunition = actor_find_ammunition(actor, weapon);

                    if (ammunition)
                    {
                        if (actor_equip(actor, ammunition))
                        {
                            return true;
                        }
                    }

                    // no ammo, so look for a melee weapon to equip
                    struct item *const melee_weapon = actor_find_melee_weapon(actor);

                    if (melee_weapon)
                    {
                        if (actor_equip(actor, melee_weapon))
                        {
                            return true;
                        }
                    }

                    // no melee weapon, so unequip the ranged weapon
                    if (actor_unequip(actor, EQUIP_SLOT_WEAPON))
                    {
                        return true;
                    }
                }

                // take the shot
                if (actor_shoot(actor, nearest_enemy->x, nearest_enemy->y))
                {
                    return true;
                }
            }
            else
            {
                // does the actor have a ranged weapon in their inventory?
                struct item *const ranged_weapon = actor_find_ranged_weapon(actor);

                if (ranged_weapon)
                {
                    // does the actor have ammo?
                    if (actor_has_ammunition(actor, ranged_weapon))
                    {
                        // equip the ranged weapon
                        if (actor_equip(actor, ranged_weapon))
                        {
                            return true;
                        }
                    }

                    // look for suitable ammo in inventory and equip it
                    struct item *const ammunition = actor_find_ammunition(actor, ranged_weapon);

                    if (ammunition)
                    {
                        if (actor_equip(actor, ammunition))
                        {
                            return true;
                        }
                    }
                }
            }

            // does the actor have a weapon?
            if (!weapon)
            {
                // look for a melee weapon to equip
                struct item *const melee_weapon = actor_find_melee_weapon(actor);

                if (melee_weapon)
                {
                    if (actor_equip(actor, melee_weapon))
                    {
                        return true;
                    }
                }
            }

            // move towards enemy
            if (actor_path_towards(actor, nearest_enemy->x, nearest_enemy->y))
            {
                return true;
            }
        }
    }
    else
    {
        actor->current_target = NULL;
    }

    // go to where a hostile target was recently seen
    if (actor->last_seen_x != -1 && actor->last_seen_y != -1)
    {
        if ((actor->x == actor->last_seen_x && actor->y == actor->last_seen_y) ||
            actor->turns_chased > actor_metadata.turns_to_chase)
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

    // stay within visiblity/proximity to leader
    if (actor->leader)
    {
        if (!TCOD_map_is_in_fov(actor->fov, actor->leader->x, actor->leader->y) ||
            distance_between(actor->x, actor->y, actor->leader->x, actor->leader->y) > 5)
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
    const struct tile *const tile = &map->tiles[actor->x][actor->y];

    if (tile->item)
    {
        if (actor_grab(actor, actor->x, actor->y))
        {
            return true;
        }
    }

    // move randomly
    if (TCOD_random_get_int(NULL, 0, 1) == 0)
    {
        const int x = actor->x + TCOD_random_get_int(NULL, -1, 1);
        const int y = actor->y + TCOD_random_get_int(NULL, -1, 1);

        actor_move(actor, x, y);

        return true;
    }

    return true;
}

bool actor_rest(struct actor *const actor)
{
    const struct map *const map = &world->maps[actor->floor];

    for (size_t actor_index = 0; actor_index < map->actors->size; actor_index++)
    {
        struct actor *const other = list_get(map->actors, actor_index);

        if (other->faction != actor->faction)
        {
            world_log(
                actor->floor,
                actor->x,
                actor->y,
                color_white,
                "%s cannot rest while enemies are nearby.",
                actor->name);

            return false;
        }
    }

    struct item *food = NULL;

    for (size_t item_index = 0; item_index < actor->items->size; item_index++)
    {
        struct item *const item = list_get(actor->items, item_index);

        if (item->type == ITEM_TYPE_FOOD)
        {
            food = item;

            break;
        }
    }

    if (!food)
    {
        world_log(
            actor->floor,
            actor->x,
            actor->y,
            color_white,
            "%s has no food to rest with.",
            actor->name);

        return false;
    }

    food->stack--;

    if (food->stack == 0)
    {
        list_remove(actor->items, food);

        item_delete(food);
    }

    actor_restore_hit_points(actor, actor_get_max_hit_points(actor) - actor->hit_points);
    actor_restore_mana(actor, actor_get_max_mana(actor) - actor->mana);

    world_log(
        actor->floor,
        actor->x,
        actor->y,
        color_white,
        "%s rests.",
        actor->name);

    return true;
}

bool actor_path_towards(
    struct actor *const actor,
    const int target_x, const int target_y)
{
    TCOD_Map *const TCOD_map = map_to_TCOD_map(&world->maps[actor->floor]);
    TCOD_map_set_properties(
        TCOD_map,
        target_x, target_y,
        TCOD_map_is_transparent(TCOD_map, target_x, target_y),
        true);

    const TCOD_path_t path = TCOD_path_new_using_map(TCOD_map, 1);
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
    const float distance = distance_between(
        actor->x, actor->y,
        target_x, target_y);

    if (distance > 0)
    {
        const int dx = (int)roundf((target_x - actor->x) / distance);
        const int dy = (int)roundf((target_y - actor->y) / distance);

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

    if (!tile_database[tile->type].is_walkable)
    {
        return false;
    }

    if (tile->object)
    {
        if (actor_interact(actor, tile->object))
        {
            return true;
        }

        if (!object_database[tile->object->type].is_walkable)
        {
            return false;
        }

        if (tile->object->type == OBJECT_TYPE_TRAP)
        {
            const int reflex_save = actor_get_saving_throw(actor, SAVING_THROW_REFLEX);
            const int roll = TCOD_random_dice_roll_s(NULL, "1d20") + reflex_save;
            const int dungeon_level = map_get_dungeon_level(map);
            const int challenge_rating = 10 + dungeon_level; // object_database[tile->object->type].challenge_rating;

            if (roll == 1 ||
                (roll != 20 &&
                 roll < challenge_rating))
            {
                const int damage = TCOD_random_dice_roll_s(NULL, "1d6") * dungeon_level;

                world_log(
                    actor->floor,
                    actor->x,
                    actor->y,
                    color_red,
                    "%s triggers a trap and takes %d damage!",
                    actor->name,
                    damage);

                actor_damage_hit_points(actor, NULL, damage);
            }
            else
            {
                world_log(
                    actor->floor,
                    actor->x,
                    actor->y,
                    color_white,
                    "%s avoids a trap!",
                    actor->name);
            }

            list_remove(map->objects, tile->object);

            object_delete(tile->object);
            tile->object = NULL;
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
            if (actor_has_ranged_weapon(actor))
            {
                return actor_shoot(actor, x, y);
            }
            else
            {
                return actor_attack(actor, tile->actor, false, NULL);
            }
        }
    }

    for (size_t actor_index = 0; actor_index < map->actors->size; actor_index++)
    {
        struct actor *const other = list_get(map->actors, actor_index);

        if (actor_can_make_attack_of_opportunity(other, actor) &&
            distance_between(x, y, other->x, other->y) >= 2)
        {
            actor_attack(other, actor, false, NULL);
        }
    }

    map->tiles[actor->x][actor->y].actor = NULL;

    tile->actor = actor;

    actor->x = x;
    actor->y = y;

    return true;
}

bool actor_swap(struct actor *const actor, struct actor *const other)
{
    // npc actors can't swap
    if (actor != world->player)
    {
        return false;
    }

    // swap actor coordinates
    const int temp_x = actor->x;
    const int temp_y = actor->y;
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
        color_white,
        "%s swaps with %s.",
        actor->name,
        other->name);

    return true;
}

bool actor_interact(struct actor *actor, struct object *object)
{
    switch (object->type)
    {
    case OBJECT_TYPE_ALTAR:
    {
        return actor_pray(actor, object->x, object->y);
    }
    break;
    case OBJECT_TYPE_BRAZIER:
    {
    }
    break;
    case OBJECT_TYPE_CHEST:
    {
        return actor_open_chest(actor, object->x, object->y);
    }
    break;
    case OBJECT_TYPE_DOOR_CLOSED:
    {
        return actor_open_door(actor, object->x, object->y);
    }
    break;
    case OBJECT_TYPE_DOOR_OPEN:
    {
    }
    break;
    case OBJECT_TYPE_FOUNTAIN:
    {
        return actor_drink(actor, object->x, object->y);
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
        return actor_sit(actor, object->x, object->y);
    }
    break;
    case OBJECT_TYPE_TRAP:
    {
    }
    break;
    }

    return false;
}

bool actor_open_door(
    struct actor *const actor,
    const int x, const int y)
{
    if (!map_is_inside(x, y))
    {
        return false;
    }

    struct tile *const tile = &world->maps[actor->floor].tiles[x][y];

    if (!tile->object || tile->object->type != OBJECT_TYPE_DOOR_CLOSED)
    {
        world_log(
            actor->floor,
            actor->x,
            actor->y,
            color_white,
            "%s can't open the door.",
            actor->name);

        return false;
    }

    tile->object->type = OBJECT_TYPE_DOOR_OPEN;

    world_log(
        actor->floor,
        actor->x,
        actor->y,
        color_orange,
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

    struct tile *const tile = &world->maps[actor->floor].tiles[x][y];

    if (!tile->object || tile->object->type != OBJECT_TYPE_DOOR_OPEN)
    {
        world_log(
            actor->floor,
            actor->x,
            actor->y,
            color_white,
            "%s can't close this.",
            actor->name);

        return false;
    }

    tile->object->type = OBJECT_TYPE_DOOR_CLOSED;

    world_log(
        actor->floor,
        actor->x,
        actor->y,
        color_orange,
        "%s closes the door.",
        actor->name);

    return true;
}

bool actor_descend(struct actor *const actor)
{
    // is there a next map?
    if (actor->floor >= NUM_MAPS - 1)
    {
        world_log(
            actor->floor,
            actor->x,
            actor->y,
            color_white,
            "%s has reached the end.",
            actor->name);

        return false;
    }

    struct map *const map = &world->maps[actor->floor];
    struct tile *const tile = &map->tiles[actor->x][actor->y];

    // is the actor on stairs down?
    if (!tile->object || tile->object->type != OBJECT_TYPE_STAIR_DOWN)
    {
        world_log(
            actor->floor,
            actor->x,
            actor->y,
            color_white,
            "%s can't descend here.",
            actor->name);

        return false;
    }

    // remove actor from current map
    list_remove(map->actors, actor);

    // remove actor from current tile
    tile->actor = NULL;

    // add actor to next map
    struct map *const next_map = &world->maps[actor->floor + 1];
    list_add(next_map->actors, actor);

    // add actor to next tile
    struct tile *const next_tile = &next_map->tiles[next_map->stair_up_x][next_map->stair_up_y];
    next_tile->actor = actor;

    // change actor coordinates
    actor->floor = next_map->floor;
    actor->x = next_map->stair_up_x;
    actor->y = next_map->stair_up_y;

    // TODO: find all followers and move them to the next map as well

    world_log(
        actor->floor,
        actor->x,
        actor->y,
        color_white,
        "%s descends.",
        actor->name);

    return true;
}

bool actor_ascend(struct actor *actor)
{
    // is there a previous map?
    if (actor->floor == 0)
    {
        world_log(
            actor->floor,
            actor->x,
            actor->y,
            color_white,
            "%s can't go any higher.",
            actor->name);

        return false;
    }

    struct map *const map = &world->maps[actor->floor];
    struct tile *const tile = &map->tiles[actor->x][actor->y];

    // is the actor on stairs up?
    if (!tile->object || tile->object->type != OBJECT_TYPE_STAIR_UP)
    {
        world_log(
            actor->floor,
            actor->x,
            actor->y,
            color_white,
            "%s can't ascend here.",
            actor->name);

        return false;
    }

    // remove actor from current map
    list_remove(map->actors, actor);

    // remove actor from current tile
    tile->actor = NULL;

    // add actor to next map
    struct map *const next_map = &world->maps[actor->floor - 1];
    list_add(next_map->actors, actor);

    // add actor to next tile
    struct tile *const next_tile = &next_map->tiles[next_map->stair_down_x][next_map->stair_down_y];
    next_tile->actor = actor;

    // change actor coordinates
    actor->floor = next_map->floor;
    actor->x = next_map->stair_down_x;
    actor->y = next_map->stair_down_y;

    // TODO: find all followers and move them to the next map as well

    world_log(
        actor->floor,
        actor->x,
        actor->y,
        color_white,
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
            color_white,
            "%s can't open the chest.",
            actor->name);

        return false;
    }

    // TODO: give item
    // this should dynamically figure out what the player might need
    // with a little randomness thrown in

    list_remove(map->objects, tile->object);

    object_delete(tile->object);
    tile->object = NULL;

    world_log(
        actor->floor,
        actor->x,
        actor->y,
        color_orange,
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
            color_white,
            "%s can't pray here.",
            actor->name);

        return false;
    }

    const enum ability ability = TCOD_random_get_int(NULL, ABILITY_NONE + 1, NUM_ABILITIES - 1);
    const int score = 1;

    actor->ability_scores[ability] += score;

    list_remove(map->objects, tile->object);

    object_delete(tile->object);
    tile->object = NULL;

    world_log(
        actor->floor,
        actor->x,
        actor->y,
        color_orange,
        "%s prays at the altar. The gods grant %d %s.",
        actor->name,
        score,
        ability_database[ability].name);

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
            color_white,
            "%s can't drink here.",
            actor->name);

        return false;
    }

    actor_restore_hit_points(actor, actor_get_max_hit_points(actor) - actor->hit_points);
    actor_restore_mana(actor, actor_get_max_mana(actor) - actor->mana);

    list_remove(map->objects, tile->object);

    object_delete(tile->object);
    tile->object = NULL;

    world_log(
        actor->floor,
        actor->x,
        actor->y,
        color_orange,
        "%s drinks from the fountain.",
        actor->name);

    return true;
}

bool actor_dip(
    struct actor *const actor,
    const int x, const int y)
{
    if (!map_is_inside(x, y))
    {
        return false;
    }

    struct tile *const tile = &world->maps[actor->floor].tiles[x][y];

    if (tile->object && tile->object->type == OBJECT_TYPE_BRAZIER)
    {
        // TODO: set weapon on fire temporarily

        return true;
    }

    if (tile->surface)
    {
        if (tile->surface->type == SURFACE_TYPE_FIRE)
        {
            // TODO: set weapon on fire temporarily
        }

        if (tile->surface->type == SURFACE_TYPE_WATER)
        {
            // TODO: extinguish weapon
        }
    }

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
            color_white,
            "%s can't sit here",
            actor->name);

        return false;
    }

    // TODO: throne effects
    // donate gold for a reward?

    list_remove(map->objects, tile->object);

    object_delete(tile->object);
    tile->object = NULL;

    world_log(
        actor->floor,
        actor->x,
        actor->y,
        color_orange,
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

    if (!tile->item)
    {
        world_log(
            actor->floor,
            actor->x,
            actor->y,
            color_white,
            "%s cannot find anything to pick up.",
            actor->name);

        return false;
    }

    if (actor->items->size >= 26)
    {
        world_log(
            actor->floor,
            actor->x,
            actor->y,
            color_white,
            "%s is carrying too many items.",
            actor->name);

        return false;
    }

    struct item *const item = tile->item;
    list_remove(map->items, item);
    tile->item = NULL;

    if (item->type == ITEM_TYPE_GOLD)
    {
        const int gold = item->stack;
        actor->gold += gold;

        item_delete(item);

        world_log(
            actor->floor,
            actor->x,
            actor->y,
            color_white,
            "%s picks up %d gold.",
            actor->name,
            gold);
    }
    else
    {
        list_add(actor->items, item);

        world_log(
            actor->floor,
            actor->x,
            actor->y,
            color_white,
            "%s picks up %s.",
            actor->name,
            item_database[item->type].name);
    }

    return true;
}

bool actor_drop(struct actor *const actor, struct item *const item)
{
    // remove from actor's inventory
    list_remove(actor->items, item);

    // change item coordinates
    item->floor = actor->floor;
    item->x = actor->x;
    item->y = actor->y;

    // find a clear tile to drop the item on
    struct map *const map = &world->maps[actor->floor];

    map_find_empty_tile(map, &item->x, &item->y);

    // add item to map
    list_add(map->items, item);

    // add item to tile
    struct tile *const tile = &map->tiles[item->x][item->y];

    tile->item = item;

    world_log(
        actor->floor,
        actor->x,
        actor->y,
        color_white,
        "%s drops %s.",
        actor->name,
        item_database[item->type].name);

    return true;
}

bool actor_equip(struct actor *const actor, struct item *const item)
{
    // is item equipment?
    const struct item_data *const item_data = &item_database[item->type];
    const struct base_item_data *const base_item_data = &base_item_database[item_data->type];

    if (base_item_data->equip_slot == EQUIP_SLOT_NONE)
    {
        world_log(
            actor->floor,
            actor->x,
            actor->y,
            color_white,
            "%s cannot equip %s.",
            actor->name,
            item_data->name);

        return false;
    }

    // is item equippable
    const enum equippability equippability = actor_get_item_equippability(actor, item);

    if (equippability == EQUIPPABILITY_TOO_LARGE)
    {
        world_log(
            actor->floor,
            actor->x,
            actor->y,
            color_white,
            "%s is too large for %s.",
            item_data->name,
            actor->name);

        return false;
    }

    if (equippability == EQUIPPABILITY_TOO_SMALL)
    {
        world_log(
            actor->floor,
            actor->x,
            actor->y,
            color_white,
            "%s is too small for %s.",
            item_data->name,
            actor->name);

        return false;
    }

    if (!actor_is_proficient(actor, item))
    {
        world_log(
            actor->floor,
            actor->x,
            actor->y,
            color_white,
            "%s is not proficient with %s.",
            actor->name,
            item_data->name);

        return false;
    }

    // unequip the current item in the slot
    if (actor->equipment[base_item_data->equip_slot])
    {
        actor_unequip(actor, base_item_data->equip_slot);
    }

    // if the item being equipped is two handed weapon, also unequip the shield
    if (base_item_data->equip_slot == EQUIP_SLOT_WEAPON)
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
    if (base_item_data->equip_slot == EQUIP_SLOT_SHIELD)
    {
        const struct item *const main_hand = actor->equipment[EQUIP_SLOT_WEAPON];

        if (main_hand)
        {
            const enum equippability main_hand_equippability = actor_get_item_equippability(actor, main_hand);

            if (main_hand_equippability == EQUIPPABILITY_BARELY)
            {
                actor_unequip(actor, EQUIP_SLOT_WEAPON);
            }
        }
    }

    // remove from inventory
    list_remove(actor->items, item);

    // add to slot
    actor->equipment[base_item_data->equip_slot] = item;

    world_log(
        actor->floor,
        actor->x,
        actor->y,
        color_white,
        "%s equips %s.",
        actor->name,
        item_data->name);

    return true;
}

bool actor_unequip(struct actor *const actor, const enum equip_slot equip_slot)
{
    // is something equipped in the slot?
    struct item *const equipment = actor->equipment[equip_slot];

    if (!equipment)
    {
        world_log(
            actor->floor,
            actor->x,
            actor->y,
            color_white,
            "%s is not equipping anything their %s slot.",
            actor->name,
            equip_slot_database[equip_slot].name);

        return false;
    }

    // add to inventory
    list_add(actor->items, equipment);

    // remove from slot
    actor->equipment[equip_slot] = NULL;

    world_log(
        actor->floor,
        actor->x,
        actor->y,
        color_white,
        "%s unequips %s.",
        actor->name,
        item_database[equipment->type].name);

    return true;
}

bool actor_quaff(struct actor *const actor, struct item *const item)
{
    // is the item quaffable?
    const struct item_data *const item_data = &item_database[item->type];

    if (item_data->type != BASE_ITEM_TYPE_POTION)
    {
        world_log(
            actor->floor,
            actor->x,
            actor->y,
            color_white,
            "%s cannot quaff %s.",
            actor->name,
            item_data->name);

        return false;
    }

    world_log(
        actor->floor,
        actor->x,
        actor->y,
        color_white,
        "%s quaffs %s.",
        actor->name,
        item_data->name);

    // cast the stored spell
    actor_cast(
        actor,
        item_data->spell_type,
        actor->x, actor->y,
        false);

    // decrement item stack
    item->stack--;

    // delete item if stack is empty
    if (item->stack <= 0)
    {
        // remove from inventory
        list_remove(actor->items, item);

        // delete the item
        item_delete(item);
    }

    return true;
}

bool actor_read(struct actor *const actor, struct item *const item, const int x, const int y)
{
    // is the item readable?
    const struct item_data *const item_data = &item_database[item->type];

    if (item_data->type != BASE_ITEM_TYPE_SCROLL)
    {
        world_log(
            actor->floor,
            actor->x,
            actor->y,
            color_white,
            "%s cannot read %s.",
            actor->name,
            item_data->name);

        return false;
    }

    world_log(
        actor->floor,
        actor->x,
        actor->y,
        color_white,
        "%s reads %s.",
        actor->name,
        item_data->name);

    // cast the stored spell
    actor_cast(
        actor,
        item_data->spell_type,
        x, y,
        false);

    // decrement item stack
    item->stack--;

    // delete item if stack is empty
    if (item->stack <= 0)
    {
        // remove from inventory
        list_remove(actor->items, item);

        // delete the item
        item_delete(item);
    }

    return true;
}

bool actor_learn(struct actor *const actor, struct item *const item)
{
    // is the item learnable?
    const struct item_data *const item_data = &item_database[item->type];

    if (item_data->type != BASE_ITEM_TYPE_SCROLL)
    {
        world_log(
            actor->floor,
            actor->x,
            actor->y,
            color_white,
            "%s cannot learn from %s.",
            actor->name,
            item_data->name);

        return false;
    }

    // TODO: data driven?
    if (actor->class != CLASS_CLERIC || actor->class != CLASS_WIZARD)
    {
        world_log(
            actor->floor,
            actor->x,
            actor->y,
            color_white,
            "%ss cannot learn spells.",
            class_database[actor->class].name);

        return false;
    }

    const struct spell_data *const spell_data = &spell_database[item_data->spell_type];

    if (actor->class == CLASS_CLERIC && spell_data->magic_type != MAGIC_TYPE_DIVINE)
    {
        world_log(
            actor->floor,
            actor->x,
            actor->y,
            color_white,
            "%ss can only learn divine spells.",
            class_database[actor->class].name);

        return false;
    }

    if (actor->class == CLASS_WIZARD && spell_data->magic_type != MAGIC_TYPE_ARCANE)
    {
        world_log(
            actor->floor,
            actor->x,
            actor->y,
            color_white,
            "%ss can only learn arcane spells.",
            class_database[actor->class].name);

        return false;
    }

    if (actor->spells[item_data->spell_type])
    {
        world_log(
            actor->floor,
            actor->x,
            actor->y,
            color_white,
            "%s already knows %s.",
            actor->name,
            spell_data->name);

        return false;
    }

    // add spell to known spells
    actor->spells[item_data->spell_type] = true;

    // remove from inventory
    list_remove(actor->items, item);

    // delete the item
    item_delete(item);

    return true;
}

bool actor_bash(struct actor *const actor, struct object *const object)
{
    // is the object destroyable?
    // TODO: make this a property on the object_data?
    const struct object_data *const object_data = &object_database[object->type];

    if (object->type == OBJECT_TYPE_STAIR_DOWN ||
        object->type == OBJECT_TYPE_STAIR_UP)
    {
        world_log(
            actor->floor,
            actor->x,
            actor->y,
            color_white,
            "%s cannot destroy the %s.",
            actor->name,
            object_data->name);

        return false;
    }

    // TODO: calculate damage properly
    // this means objects should have health
    // move damage calculation in actor_attack to a function and call it here as well

    // remove from map
    struct map *const map = &world->maps[actor->floor];
    list_remove(map->objects, object);

    // remove from tile
    struct tile *const tile = &map->tiles[object->x][object->y];
    tile->object = NULL;

    // delete the object
    // TODO: mark for delete
    object_delete(object);

    world_log(
        actor->floor,
        actor->x,
        actor->y,
        color_white,
        "%s destroys the %s.",
        actor->name,
        object_data->name);

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
    struct item *const weapon = actor->equipment[EQUIP_SLOT_WEAPON];

    if (!weapon)
    {
        world_log(
            actor->floor,
            actor->x,
            actor->y,
            color_white,
            "%s cannot shoot without a weapon!",
            actor->name);

        return false;
    }

    // is the weapon ranged?
    const struct base_item_data *const base_weapon_data = &base_item_database[item_database[weapon->type].type];

    if (!base_weapon_data->ranged)
    {
        world_log(
            actor->floor,
            actor->x,
            actor->y,
            color_white,
            "%s cannot shoot without a ranged weapon!",
            actor->name);

        return false;
    }

    // does the weapon require ammo?
    struct item *ammunition;
    if (base_weapon_data->ammunition_type == AMMUNITION_TYPE_NONE)
    {
        ammunition = weapon;
    }
    else
    {
        // is there ammo?
        ammunition = actor->equipment[EQUIP_SLOT_AMMUNITION];

        if (!ammunition)
        {
            world_log(
                actor->floor,
                actor->x,
                actor->y,
                color_white,
                "%s has no ammunition equipped!",
                actor->name);

            return false;
        }

        // is it the right ammo?
        if (base_item_database[item_database[ammunition->type].type].ammunition_type != base_weapon_data->ammunition_type)
        {
            world_log(
                actor->floor,
                actor->x,
                actor->y,
                color_white,
                "%s has unsuitable ammunition equipped!",
                actor->name);

            return false;
        }
    }

    // create a projectile
    // the projectile gets a copy of the ammunition with the stack size set to 1
    // this is so that if the actor deletes its ammunition item (e.g. if it runs out), the projectile can still use the data
    struct projectile *const projectile = projectile_new(
        PROJECTILE_TYPE_ARROW,
        actor->floor,
        actor->x, actor->y,
        x, y,
        actor,
        item_new(
            ammunition->type,
            ammunition->floor,
            ammunition->x,
            ammunition->y,
            1),
        0);

    // add the projectile to the map
    const struct map *const map = &world->maps[actor->floor];
    list_add(map->projectiles, projectile);

    // decrement the actor's ammunition
    ammunition->stack--;

    // unequip and delete the item if out of ammo
    if (ammunition->stack <= 0)
    {
        // remove from slot
        actor->equipment[base_item_database[item_database[ammunition->type].type].equip_slot] = NULL;

        // delete the item
        item_delete(ammunition);
    }

    return true;
}

bool actor_attack(struct actor *const actor, struct actor *const other, const bool opportunity, int *const damage_dealt)
{
    // remember the target
    actor->last_attacked_target = other;

    // calculate other armor class
    const int armor_class = actor_get_armor_class(other);

    // calculate feats
    const struct actor_feats actor_feats = actor_get_feats(actor);

    // calculate number of attacks
    const int attacks_per_round =
        opportunity
            ? actor_feats.has[FEAT_COMBAT_REFLEXES]
                  ? actor_get_ability_modifer(actor, ABILITY_DEXTERITY)
                  : 1
            : actor_get_attacks_per_round(actor);

    for (int attack = 0; attack < attacks_per_round; attack++)
    {
        // calculate hit
        const int attack_roll = TCOD_random_dice_roll_s(NULL, "1d20");
        const int attack_bonus = actor_get_attack_bonus(actor);
        const int ranged_attack_penalty = actor_get_ranged_attack_penalty(actor, other);
        const int successive_attack_penalty = attack * actor_get_successive_attack_penalty(actor);
        const int hit_challenge = attack_roll + attack_bonus - ranged_attack_penalty - successive_attack_penalty;

        if (attack_roll == 1 ||
            (attack_roll != 20 && hit_challenge < armor_class))
        {
            other->flash_color = color_gray;
            other->flash_alpha = 1;

            world_log(
                actor->floor,
                actor->x,
                actor->y,
                color_light_gray,
                "%s misses %s.",
                actor->name,
                other->name);

            if (damage_dealt)
            {
                *damage_dealt = 0;
            }

            return true;
        }

        // calculate critical hit
        bool crit = false;

        if (attack_roll >= actor_get_threat_range(actor))
        {
            const int threat_roll = TCOD_random_dice_roll_s(NULL, "1d20");
            const int crit_challenge = threat_roll + attack_bonus;

            if (crit_challenge >= armor_class)
            {
                crit = true;
            }
        }

        // calculate damage
        const char *const damage_die = actor_get_damage(actor);
        const int damage_bonus = actor_get_damage_bonus(actor);
        const int num_attack_rolls =
            crit
                ? actor_get_critical_multiplier(actor)
                : 1;

        int damage = 0;

        for (size_t current_roll = 0; current_roll < num_attack_rolls; current_roll++)
        {
            const int weapon_damage = TCOD_random_dice_roll_s(NULL, damage_die);

            damage += weapon_damage + damage_bonus;
        }

        // sneak attack
        bool sneak_attack = false;

        if (actor_feats.has[FEAT_SNEAK_ATTACK] && other->last_attacked_target != actor)
        {
            sneak_attack = true;

            for (size_t current_roll = 0; current_roll < actor->level / 2; current_roll++)
            {
                damage += TCOD_random_dice_roll_s(NULL, "1d6");
            }
        }

        // if it's a hit, it has to do damage
        if (damage < 1)
        {
            damage = 1;
        }

        // adamantine breastplate
        const struct item *const armor = actor->equipment[EQUIP_SLOT_ARMOR];

        if (armor && armor->type == ITEM_TYPE_ADAMANTINE_BREASTPLATE)
        {
            damage -= 2;

            if (damage < 0)
            {
                damage = 0;
            }
        }

        // TODO: when projectiles come at the player from the dark, nothing gets logged
        // it'd be nice if there were a way to do something like "someone attacks <player> for <damage>"
        world_log(
            actor->floor,
            actor->x,
            actor->y,
            crit ? color_light_red : color_white,
            "%s %s %s for %d%s%s.",
            actor->name,
            crit ? "crits" : "hits",
            other->name,
            damage,
            opportunity ? " (opportunity)" : "",
            sneak_attack ? " (sneak)" : "");

        // weapon breakage
        bool weapon_broken = false;
        struct item *const weapon = actor->equipment[EQUIP_SLOT_WEAPON];
        const struct item_data *const weapon_data = weapon ? &item_database[weapon->type] : NULL;

        if (weapon && weapon_data->breakable &&
            TCOD_random_get_float(NULL, 0, 1) <= 0.01f)
        {
            weapon_broken = true;

            world_log(
                actor->floor,
                actor->x,
                actor->y,
                color_red,
                "%s's %s breaks!",
                actor->name,
                weapon_data->name);

            actor->equipment[EQUIP_SLOT_WEAPON] = NULL;

            item_delete(weapon);
        }

        // deal damage
        const bool killed = actor_damage_hit_points(other, actor, damage);

        if (damage_dealt)
        {
            *damage_dealt += damage;
        }

        if (weapon_broken)
        {
            break;
        }

        if (killed)
        {
            // TODO: this currently acts like great cleave, need to limit to only 1 attack per turn
            if (actor_feats.has[FEAT_CLEAVE])
            {
                const struct map *const map = &world->maps[actor->floor];

                for (int actor_index = 0; actor_index < map->actors->size; actor_index++)
                {
                    struct actor *const target = list_get(map->actors, actor_index);

                    if (target != other &&
                        actor_is_enemy(actor, target) &&
                        distance_between(actor->x, actor->y, target->x, target->y) < 2)
                    {
                        actor_attack(actor, target, false, NULL);

                        break;
                    }
                }
            }

            break;
        }
    }

    return true;
}

bool actor_cast(
    struct actor *const actor,
    const enum spell_type spell_type,
    const int x, const int y,
    const bool from_memory)
{
    if (!map_is_inside(x, y))
    {
        return false;
    }

    const struct spell_data *const spell_data = &spell_database[spell_type];
    const int mana_cost = actor_get_spell_mana_cost(actor, spell_type);

    if (from_memory)
    {
        const struct actor_spells actor_spells = actor_get_spells(actor);

        // does the actor know the spell?
        if (!actor_spells.has[spell_type])
        {
            world_log(
                actor->floor,
                actor->x,
                actor->y,
                color_white,
                "%s does not know %s.",
                actor->name,
                spell_data->name);

            return false;
        }

        // is the actor high enough level?
        if (actor->level < spell_data->level)
        {
            world_log(
                actor->floor,
                actor->x,
                actor->y,
                color_white,
                "%s is not high enough level to cast %s.",
                actor->name,
                spell_data->name);

            return false;
        }

        // does the actor have enough mana?
        if (actor->mana < mana_cost)
        {
            world_log(
                actor->floor,
                actor->x,
                actor->y,
                color_white,
                "%s does not have enough mana.",
                actor->name,
                spell_data->name);

            return false;
        }

        // provoke attack of opportunity and check concentration
        bool interrupted = false;
        struct list *damages = list_new();

        actor_provoke_attack_of_opportunity(actor, &damages);

        if (damages->size)
        {
            const struct actor_feats actor_feats = actor_get_feats(actor);

            for (size_t damage_index = 0; damage_index < damages->size; damage_index++)
            {
                const int damage = (int)(intptr_t)list_get(damages, damage_index);
                const int concentration_check = 10 + damage;
                const int concentration_roll =
                    TCOD_random_dice_roll_s(NULL, "1d20") +
                    actor_get_ability_modifer(actor, ABILITY_CONSTITUTION) +
                    (actor_feats.has[FEAT_COMBAT_CASTING] ? 4 : 0);

                if (concentration_roll < concentration_check)
                {
                    interrupted = true;

                    break;
                }
            }
        }

        list_delete(damages);

        if (actor->dead)
        {
            return false;
        }

        if (interrupted)
        {
            world_log(
                actor->floor,
                actor->x,
                actor->y,
                color_red,
                "%s's spell is interrupted.",
                actor->name,
                spell_data->name);

            actor->mana -= mana_cost;

            return true;
        }

        // arcane spell failure
        const float arcane_spell_failure = actor_get_arcane_spell_failure(actor);

        if (arcane_spell_failure > 0 && spell_data->magic_type == MAGIC_TYPE_ARCANE)
        {
            const float roll = TCOD_random_get_float(NULL, 0, 1);

            if (roll < arcane_spell_failure)
            {
                world_log(
                    actor->floor,
                    actor->x,
                    actor->y,
                    color_red,
                    "%s's spell fizzles.",
                    actor->name,
                    spell_data->name);

                actor->mana -= mana_cost;

                return true;
            }
        }
    }

    world_log(
        actor->floor,
        actor->x,
        actor->y,
        color_purple,
        "%s casts %s.",
        actor->name,
        spell_data->name);

    int caster_level = spell_data->level;

    if (from_memory)
    {
        actor->mana -= mana_cost;

        caster_level = actor->level;
    }

    switch (spell_type)
    {
    case SPELL_TYPE_ACID_SPLASH:
    {
        if (x == actor->x && y == actor->y)
        {
            return false;
        }

        // create a projectile
        struct projectile *const projectile = projectile_new(
            PROJECTILE_TYPE_ACID_SPLASH,
            actor->floor,
            actor->x,
            actor->y,
            x,
            y,
            actor,
            NULL,
            caster_level);

        // add the projectile to the map
        list_add(world->maps[actor->floor].projectiles, projectile);
    }
    break;
    case SPELL_TYPE_CURE_MINOR_WOUNDS:
    {
        struct actor *const other = world->maps[actor->floor].tiles[x][y].actor;

        if (other)
        {
            if (other->race == RACE_UNDEAD)
            {
                if (actor_ranged_touch_attack(actor, other))
                {
                    world_log(
                        other->floor,
                        other->x,
                        other->y,
                        color_white,
                        "%s damages %s for %d.",
                        actor->name,
                        other->name,
                        1);

                    actor_damage_hit_points(other, actor, 1);
                }
                else
                {
                    world_log(
                        actor->floor,
                        actor->x,
                        actor->y,
                        color_light_gray,
                        "%s misses %s.",
                        actor->name,
                        other->name);
                }
            }
            else
            {
                world_log(
                    other->floor,
                    other->x,
                    other->y,
                    color_white,
                    "%s heals %s for 1.",
                    actor->name,
                    other->name,
                    1);

                actor_restore_hit_points(other, 1);
            }
        }
    }
    break;
    case SPELL_TYPE_CURE_LIGHT_WOUNDS:
    {
        struct actor *const other = world->maps[actor->floor].tiles[x][y].actor;

        if (other)
        {
            const int health = TCOD_random_dice_roll_s(NULL, "1d8") + MIN(caster_level, 5);

            if (other->race == RACE_UNDEAD)
            {
                if (actor_ranged_touch_attack(actor, other))
                {
                    world_log(
                        other->floor,
                        other->x,
                        other->y,
                        color_white,
                        "%s damages %s for %d.",
                        actor->name,
                        other->name,
                        health);

                    actor_damage_hit_points(other, actor, health);
                }
                else
                {
                    world_log(
                        actor->floor,
                        actor->x,
                        actor->y,
                        color_light_gray,
                        "%s misses %s.",
                        actor->name,
                        other->name);
                }
            }
            else
            {
                world_log(
                    other->floor,
                    other->x,
                    other->y,
                    color_white,
                    "%s heals %s for %d.",
                    actor->name,
                    other->name,
                    health);

                actor_restore_hit_points(other, health);
            }
        }
    }
    break;
    case SPELL_TYPE_CURE_MODERATE_WOUNDS:
    {
        struct actor *const other = world->maps[actor->floor].tiles[x][y].actor;

        if (other)
        {
            const int health = TCOD_random_dice_roll_s(NULL, "2d8") + MIN(caster_level, 10);

            if (other->race == RACE_UNDEAD)
            {
                if (actor_ranged_touch_attack(actor, other))
                {
                    world_log(
                        other->floor,
                        other->x,
                        other->y,
                        color_white,
                        "%s damages %s for %d.",
                        actor->name,
                        other->name,
                        health);

                    actor_damage_hit_points(other, actor, health);
                }
                else
                {
                    world_log(
                        actor->floor,
                        actor->x,
                        actor->y,
                        color_light_gray,
                        "%s misses %s.",
                        actor->name,
                        other->name);
                }
            }
            else
            {
                world_log(
                    other->floor,
                    other->x,
                    other->y,
                    color_white,
                    "%s heals %s for %d.",
                    actor->name,
                    other->name,
                    health);

                actor_restore_hit_points(other, health);
            }
        }
    }
    break;
    case SPELL_TYPE_CURE_SERIOUS_WOUNDS:
    {
        struct actor *const other = world->maps[actor->floor].tiles[x][y].actor;

        if (other)
        {
            const int health = TCOD_random_dice_roll_s(NULL, "3d8") + MIN(caster_level, 15);

            if (other->race == RACE_UNDEAD)
            {
                if (actor_ranged_touch_attack(actor, other))
                {
                    world_log(
                        other->floor,
                        other->x,
                        other->y,
                        color_white,
                        "%s damages %s for %d.",
                        actor->name,
                        other->name,
                        health);

                    actor_damage_hit_points(other, actor, health);
                }
                else
                {
                    world_log(
                        actor->floor,
                        actor->x,
                        actor->y,
                        color_light_gray,
                        "%s misses %s.",
                        actor->name,
                        other->name);
                }
            }
            else
            {
                world_log(
                    other->floor,
                    other->x,
                    other->y,
                    color_white,
                    "%s heals %s for %d.",
                    actor->name,
                    other->name,
                    health);

                actor_restore_hit_points(other, health);
            }
        }
    }
    break;
    case SPELL_TYPE_CURE_CRITICAL_WOUNDS:
    {
        struct actor *const other = world->maps[actor->floor].tiles[x][y].actor;

        if (other)
        {
            const int health = TCOD_random_dice_roll_s(NULL, "4d8") + MIN(caster_level, 20);

            if (other->race == RACE_UNDEAD)
            {
                if (actor_ranged_touch_attack(actor, other))
                {
                    world_log(
                        other->floor,
                        other->x,
                        other->y,
                        color_white,
                        "%s damages %s for %d.",
                        actor->name,
                        other->name,
                        health);

                    actor_damage_hit_points(other, actor, health);
                }
                else
                {
                    world_log(
                        actor->floor,
                        actor->x,
                        actor->y,
                        color_light_gray,
                        "%s misses %s.",
                        actor->name,
                        other->name);
                }
            }
            else
            {
                world_log(
                    other->floor,
                    other->x,
                    other->y,
                    color_white,
                    "%s heals %s for %d.",
                    actor->name,
                    other->name,
                    health);

                actor_restore_hit_points(other, health);
            }
        }
    }
    break;
    case SPELL_TYPE_CHAIN_LIGHTNING:
    {
        if (x == actor->x && y == actor->y)
        {
            return false;
        }

        struct actor *const other = world->maps[actor->floor].tiles[x][y].actor;

        if (other)
        {
            // TODO: spread to nearby enemies
            TCOD_dice_t dice = TCOD_random_dice_new("1d6");
            dice.nb_rolls = MIN(caster_level, 20);

            const int damage = TCOD_random_dice_roll(NULL, dice);

            world_log(
                actor->floor,
                actor->x,
                actor->y,
                color_white,
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
                color_white,
                "%s cannot cast %s here.",
                actor->name,
                spell_data->name);

            return false;
        }
    }
    break;
    case SPELL_TYPE_DAZE:
    {
        if (x == actor->x && y == actor->y)
        {
            return false;
        }

        struct actor *const other = world->maps[actor->floor].tiles[x][y].actor;

        if (other)
        {
            // TODO: daze effect

            world_log(
                actor->floor,
                actor->x,
                actor->y,
                color_white,
                "%s dazes %s.",
                actor->name,
                other->name);
        }
        else
        {
            world_log(
                actor->floor,
                actor->x,
                actor->y,
                color_white,
                "%s cannot cast %s here.",
                actor->name,
                spell_data->name);

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

        // create a projectile
        struct projectile *const projectile = projectile_new(
            PROJECTILE_TYPE_FIREBALL,
            actor->floor,
            actor->x,
            actor->y,
            x,
            y,
            actor,
            NULL,
            caster_level);

        // add the projectile to the map
        list_add(world->maps[actor->floor].projectiles, projectile);
    }
    break;
    case SPELL_TYPE_HARM:
    {
        if (x == actor->x && y == actor->y)
        {
            return false;
        }

        struct actor *const other = world->maps[actor->floor].tiles[x][y].actor;

        if (other)
        {
            if (other->race == RACE_UNDEAD)
            {
                const int health = actor_get_max_hit_points(other) - other->hit_points;

                world_log(
                    other->floor,
                    other->x,
                    other->y,
                    color_white,
                    "%s heals %s for %d.",
                    actor->name,
                    other->name,
                    health);

                actor_restore_hit_points(other, health);
            }
            else
            {
                if (actor_ranged_touch_attack(actor, other))
                {
                    const int health = TCOD_random_dice_roll_s(NULL, "1d4");
                    const int damage = other->hit_points - health;

                    world_log(
                        other->floor,
                        other->x,
                        other->y,
                        color_white,
                        "%s damages %s for %d.",
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
                        color_light_gray,
                        "%s misses %s.",
                        actor->name,
                        other->name);
                }
            }
        }
    }
    break;
    case SPELL_TYPE_HEAL:
    {
        struct actor *const other = world->maps[actor->floor].tiles[x][y].actor;

        if (other)
        {
            if (other->race == RACE_UNDEAD)
            {
                if (actor_ranged_touch_attack(actor, other))
                {
                    const int health = TCOD_random_dice_roll_s(NULL, "1d4");
                    const int damage = other->hit_points - health;

                    world_log(
                        other->floor,
                        other->x,
                        other->y,
                        color_white,
                        "%s damages %s for %d.",
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
                        color_light_gray,
                        "%s misses %s.",
                        actor->name,
                        other->name);
                }
            }
            else
            {
                const int health = actor_get_max_hit_points(other) - other->hit_points;

                world_log(
                    other->floor,
                    other->x,
                    other->y,
                    color_white,
                    "%s heals %s for %d.",
                    actor->name,
                    other->name,
                    health);

                actor_restore_hit_points(other, health);
            }
        }
    }
    break;
    case SPELL_TYPE_INFLICT_MINOR_WOUNDS:
    {
        if (x == actor->x && y == actor->y)
        {
            return false;
        }

        struct actor *const other = world->maps[actor->floor].tiles[x][y].actor;

        if (other)
        {
            if (other->race == RACE_UNDEAD)
            {
                world_log(
                    other->floor,
                    other->x,
                    other->y,
                    color_white,
                    "%s heals %s for 1.",
                    actor->name,
                    other->name,
                    1);

                actor_restore_hit_points(other, 1);
            }
            else
            {
                if (actor_ranged_touch_attack(actor, other))
                {
                    world_log(
                        other->floor,
                        other->x,
                        other->y,
                        color_white,
                        "%s damages %s for %d.",
                        actor->name,
                        other->name,
                        1);

                    actor_damage_hit_points(other, actor, 1);
                }
                else
                {
                    world_log(
                        actor->floor,
                        actor->x,
                        actor->y,
                        color_light_gray,
                        "%s misses %s.",
                        actor->name,
                        other->name);
                }
            }
        }
    }
    break;
    case SPELL_TYPE_INFLICT_LIGHT_WOUNDS:
    {
        if (x == actor->x && y == actor->y)
        {
            return false;
        }

        struct actor *const other = world->maps[actor->floor].tiles[x][y].actor;

        if (other)
        {
            const int health = TCOD_random_dice_roll_s(NULL, "1d8") + MIN(caster_level, 5);

            if (other->race == RACE_UNDEAD)
            {
                world_log(
                    other->floor,
                    other->x,
                    other->y,
                    color_white,
                    "%s heals %s for %d.",
                    actor->name,
                    other->name,
                    health);

                actor_restore_hit_points(other, health);
            }
            else
            {
                if (actor_ranged_touch_attack(actor, other))
                {
                    world_log(
                        other->floor,
                        other->x,
                        other->y,
                        color_white,
                        "%s damages %s for %d.",
                        actor->name,
                        other->name,
                        health);

                    actor_damage_hit_points(other, actor, health);
                }
                else
                {
                    world_log(
                        actor->floor,
                        actor->x,
                        actor->y,
                        color_light_gray,
                        "%s misses %s.",
                        actor->name,
                        other->name);
                }
            }
        }
    }
    break;
    case SPELL_TYPE_INFLICT_MODERATE_WOUNDS:
    {
        if (x == actor->x && y == actor->y)
        {
            return false;
        }

        struct actor *const other = world->maps[actor->floor].tiles[x][y].actor;

        if (other)
        {
            const int health = TCOD_random_dice_roll_s(NULL, "2d8") + MIN(caster_level, 10);

            if (other->race == RACE_UNDEAD)
            {
                world_log(
                    other->floor,
                    other->x,
                    other->y,
                    color_white,
                    "%s heals %s for %d.",
                    actor->name,
                    other->name,
                    health);

                actor_restore_hit_points(other, health);
            }
            else
            {
                if (actor_ranged_touch_attack(actor, other))
                {
                    world_log(
                        other->floor,
                        other->x,
                        other->y,
                        color_white,
                        "%s damages %s for %d.",
                        actor->name,
                        other->name,
                        health);

                    actor_damage_hit_points(other, actor, health);
                }
                else
                {
                    world_log(
                        actor->floor,
                        actor->x,
                        actor->y,
                        color_light_gray,
                        "%s misses %s.",
                        actor->name,
                        other->name);
                }
            }
        }
    }
    break;
    case SPELL_TYPE_INFLICT_SERIOUS_WOUNDS:
    {
        if (x == actor->x && y == actor->y)
        {
            return false;
        }

        struct actor *const other = world->maps[actor->floor].tiles[x][y].actor;

        if (other)
        {
            const int health = TCOD_random_dice_roll_s(NULL, "3d8") + MIN(caster_level, 15);

            if (other->race == RACE_UNDEAD)
            {
                world_log(
                    other->floor,
                    other->x,
                    other->y,
                    color_white,
                    "%s heals %s for %d.",
                    actor->name,
                    other->name,
                    health);

                actor_restore_hit_points(other, health);
            }
            else
            {
                if (actor_ranged_touch_attack(actor, other))
                {
                    world_log(
                        other->floor,
                        other->x,
                        other->y,
                        color_white,
                        "%s damages %s for %d.",
                        actor->name,
                        other->name,
                        health);

                    actor_damage_hit_points(other, actor, health);
                }
                else
                {
                    world_log(
                        actor->floor,
                        actor->x,
                        actor->y,
                        color_light_gray,
                        "%s misses %s.",
                        actor->name,
                        other->name);
                }
            }
        }
    }
    break;
    case SPELL_TYPE_INFLICT_CRITICAL_WOUNDS:
    {
        if (x == actor->x && y == actor->y)
        {
            return false;
        }

        struct actor *const other = world->maps[actor->floor].tiles[x][y].actor;

        if (other)
        {
            const int health = TCOD_random_dice_roll_s(NULL, "4d8") + MIN(caster_level, 20);

            if (other->race == RACE_UNDEAD)
            {
                world_log(
                    other->floor,
                    other->x,
                    other->y,
                    color_white,
                    "%s heals %s for %d.",
                    actor->name,
                    other->name,
                    health);

                actor_restore_hit_points(other, health);
            }
            else
            {
                if (actor_ranged_touch_attack(actor, other))
                {
                    world_log(
                        other->floor,
                        other->x,
                        other->y,
                        color_white,
                        "%s damages %s for %d.",
                        actor->name,
                        other->name,
                        health);

                    actor_damage_hit_points(other, actor, health);
                }
                else
                {
                    world_log(
                        actor->floor,
                        actor->x,
                        actor->y,
                        color_light_gray,
                        "%s misses %s.",
                        actor->name,
                        other->name);
                }
            }
        }
    }
    break;
    case SPELL_TYPE_MAGIC_MISSILE:
    {
        if (x == actor->x && y == actor->y)
        {
            return false;
        }

        // create a projectile
        struct projectile *const projectile = projectile_new(
            PROJECTILE_TYPE_MAGIC_MISSILE,
            actor->floor,
            actor->x,
            actor->y,
            x,
            y,
            actor,
            NULL,
            caster_level);

        // add the projectile to the map
        list_add(world->maps[actor->floor].projectiles, projectile);
    }
    break;
    case SPELL_TYPE_RAY_OF_FROST:
    {
        if (x == actor->x && y == actor->y)
        {
            return false;
        }

        struct actor *const other = world->maps[actor->floor].tiles[x][y].actor;

        if (other)
        {
            const int damage = TCOD_random_dice_roll_s(NULL, "1d3");

            world_log(
                actor->floor,
                actor->x,
                actor->y,
                color_white,
                "%s freezes %s for %d.",
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
                color_white,
                "%s cannot cast %s here.",
                actor->name,
                spell_data->name);

            return false;
        }
    }
    break;
    case SPELL_TYPE_RESTORE_MANA:
    {
        struct actor *const other = world->maps[actor->floor].tiles[x][y].actor;

        if (other)
        {
            const int mana = actor_get_max_mana(other) - other->mana;

            world_log(
                other->floor,
                other->x,
                other->y,
                color_white,
                "%s recovers %d mana.",
                other->name,
                mana);

            actor_restore_mana(other, mana);
        }
    }
    break;
    }

    return true;
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
        color_red,
        "%s dies.",
        actor->name);

    if (killer)
    {
        const int experience = actor->level * 100;

        actor_give_experience(killer, experience);

        if (killer->leader)
        {
            actor_give_experience(killer->leader, experience);
        }
    }
}
