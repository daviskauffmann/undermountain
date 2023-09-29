#include "actor.h"

#include "ammunition_type.h"
#include "color.h"
#include "equippability.h"
#include "explosion.h"
#include "item.h"
#include "object.h"
#include "projectile.h"
#include "room.h"
#include "spell.h"
#include "surface.h"
#include "util.h"
#include "world.h"
#include <float.h>
#include <malloc.h>
#include <math.h>
#include <string.h>

const struct race_data race_database[] = {
    // player races
    [RACE_DWARF] = {
        .name = "Dwarf",

        .size = SIZE_SMALL,
    },
    [RACE_ELF] = {
        .name = "Elf",

        .size = SIZE_MEDIUM,

        .special_abilities = {
            [SPECIAL_ABILITY_LOW_LIGHT_VISION] = true,
        },

        .feats = {
            [FEAT_WEAPON_PROFICIENCY_ELF] = true,
        },
    },
    [RACE_HUMAN] = {
        .name = "Human",

        .size = SIZE_MEDIUM,

        .feats = {
            [FEAT_QUICK_TO_MASTER] = true,
        },
    },

    // monster races
    [RACE_ANIMAL_DIMINUTIVE] = {
        .name = "Diminutive Animal",

        .size = SIZE_DIMINUTIVE,
    },
    [RACE_ANIMAL_TINY] = {
        .name = "Small Animal",

        .size = SIZE_SMALL,
    },
    [RACE_ANIMAL_SMALL] = {
        .name = "Small Animal",

        .size = SIZE_TINY,
    },
    [RACE_DRAGON_ADULT] = {
        .name = "Wyrmling Dragon",

        .size = SIZE_MEDIUM,
    },
    [RACE_DRAGON_ADULT] = {
        .name = "Adult Dragon",

        .size = SIZE_HUGE,
    },
    [RACE_DRAGON_ADULT] = {
        .name = "Ancient Dragon",

        .size = SIZE_COLOSSAL,
    },
    [RACE_GIANT_LARGE] = {
        .name = "Large Giant",

        .size = SIZE_LARGE,
    },
    [RACE_HUMANOID_SMALL] = {
        .name = "Small Humanoid",

        .size = SIZE_SMALL,
    },
    [RACE_HUMANOID_MEDIUM] = {
        .name = "Medium Humanoid",

        .size = SIZE_MEDIUM,
    },
    [RACE_UNDEAD] = {
        .name = "Undead",

        .size = SIZE_MEDIUM,
    },
};

const struct class_data class_database[] = {
    // player classes
    [CLASS_FIGHTER] = {
        .name = "Fighter",
        .color = {COLOR_BRASS},
        .glyph = '@',

        .hit_die = "1d10",

        .base_attack_bonus_progression = BASE_ATTACK_BONUS_PROGRESSION_COMBAT,

        .default_ability_scores = {
            [ABILITY_STRENGTH] = 15,
            [ABILITY_DEXTERITY] = 12,
            [ABILITY_CONSTITUTION] = 14,
            [ABILITY_INTELLIGENCE] = 10,
        },

        .feat_progression = {
            [FEAT_ARMOR_PROFICIENCY_LIGHT] = 1,
            [FEAT_ARMOR_PROFICIENCY_MEDIUM] = 1,
            [FEAT_ARMOR_PROFICIENCY_HEAVY] = 1,
            [FEAT_SHIELD_PROFICIENCY] = 1,
            [FEAT_WEAPON_PROFICIENCY_SIMPLE] = 1,
            [FEAT_WEAPON_PROFICIENCY_MARTIAL] = 1,
        },

        .starting_equipment = {
            [EQUIP_SLOT_ARMOR] = ITEM_TYPE_BREASTPLATE,
            [EQUIP_SLOT_SHIELD] = ITEM_TYPE_HEAVY_SHIELD,
            [EQUIP_SLOT_WEAPON] = ITEM_TYPE_LONGSWORD,
        },

        .starting_items = {
            [ITEM_TYPE_FOOD] = 10,
        },
    },
    [CLASS_ROGUE] = {
        .name = "Rogue",
        .color = {COLOR_YELLOW},
        .glyph = '@',

        .hit_die = "1d6",

        .base_attack_bonus_progression = BASE_ATTACK_BONUS_PROGRESSION_MIDDLE,

        .default_ability_scores = {
            [ABILITY_STRENGTH] = 14,
            [ABILITY_DEXTERITY] = 15,
            [ABILITY_CONSTITUTION] = 12,
            [ABILITY_INTELLIGENCE] = 10,
        },

        .feat_progression = {
            [FEAT_ARMOR_PROFICIENCY_LIGHT] = 1,
            [FEAT_SNEAK_ATTACK] = 1,
            [FEAT_WEAPON_PROFICIENCY_ROGUE] = 1,
        },

        .starting_equipment = {
            [EQUIP_SLOT_AMMUNITION] = ITEM_TYPE_ARROW,
            [EQUIP_SLOT_ARMOR] = ITEM_TYPE_LEATHER_ARMOR,
            [EQUIP_SLOT_WEAPON] = ITEM_TYPE_LONGBOW,
        },

        .starting_items = {
            [ITEM_TYPE_DAGGER] = 1,
            [ITEM_TYPE_FOOD] = 10,
        },
    },
    [CLASS_WIZARD] = {
        .name = "Wizard",
        .color = {COLOR_AZURE},
        .glyph = '@',

        .hit_die = "1d4",

        .base_attack_bonus_progression = BASE_ATTACK_BONUS_PROGRESSION_NON_COMBAT,

        .default_ability_scores = {
            [ABILITY_STRENGTH] = 10,
            [ABILITY_DEXTERITY] = 14,
            [ABILITY_CONSTITUTION] = 12,
            [ABILITY_INTELLIGENCE] = 15,
        },

        .feat_progression = {
            [FEAT_WEAPON_PROFICIENCY_WIZARD] = 1,
        },

        .spell_progression = {
            [SPELL_TYPE_ACID_SPLASH] = 1,
            [SPELL_TYPE_FIREBALL] = 1,
            [SPELL_TYPE_SUMMON_FAMILIAR] = 1,
            [SPELL_TYPE_LIGHTNING] = 2,
        },

        .starting_equipment = {
            [EQUIP_SLOT_ARMOR] = ITEM_TYPE_WIZARDS_ROBE,
            [EQUIP_SLOT_WEAPON] = ITEM_TYPE_DAGGER,
        },

        .starting_items = {
            [ITEM_TYPE_FOOD] = 10,
        },
    },

    // monster classes
    [CLASS_BAT] = {
        .name = "Bat",
        .color = {COLOR_GRAY},
        .glyph = 'b',

        .hit_die = "1d2",

        .natural_weapon_type = NATURAL_WEAPON_TYPE_BITE,
    },
    [CLASS_BUGBEAR] = {
        .name = "Bugbear",
        .color = {COLOR_BRASS},
        .glyph = 'b',

        .hit_die = "1d8",

        .natural_armor_bonus = 3,

        .base_attack_bonus = 2,
    },
    [CLASS_DIRE_RAT] = {
        .name = "Dire Rat",
        .color = {COLOR_GRAY},
        .glyph = 'R',

        .hit_die = "1d8",

        .natural_armor_bonus = 1,

        .natural_weapon_type = NATURAL_WEAPON_TYPE_BITE,
    },
    [CLASS_DOG] = {
        .name = "Dog",
        .color = {COLOR_LIGHTEST_GRAY},
        .glyph = 'd',

        .hit_die = "1d8",

        .natural_armor_bonus = 1,

        .natural_weapon_type = NATURAL_WEAPON_TYPE_BITE,
    },
    [CLASS_HYENA] = {
        .name = "Hyena",
        .color = {COLOR_DARK_AMBER},
        .glyph = 'h',

        .hit_die = "1d8",

        .base_attack_bonus = 1,

        .natural_armor_bonus = 1,

        .natural_weapon_type = NATURAL_WEAPON_TYPE_BITE,
    },
    [CLASS_GNOLL] = {
        .name = "Gnoll",
        .color = {COLOR_DARK_AMBER},
        .glyph = 'g',

        .hit_die = "1d8",

        .base_attack_bonus = 1,

        .natural_armor_bonus = 1,
    },
    [CLASS_GOBLIN] = {
        .name = "Goblin",
        .color = {COLOR_DARK_GREEN},
        .glyph = 'g',

        .hit_die = "1d8",

        .base_attack_bonus = 1,
    },
    [CLASS_HOBGOBLIN] = {
        .name = "Hobgoblin",
        .color = {COLOR_LIGHT_CRIMSON},
        .glyph = 'h',

        .hit_die = "1d8",

        .base_attack_bonus = 1,
    },
    [CLASS_KOBOLD] = {
        .name = "Kobold",
        .color = {COLOR_LIGHT_GRAY},
        .glyph = 'k',

        .hit_die = "1d8",

        .base_attack_bonus = 1,

        .natural_armor_bonus = 1,
    },
    [CLASS_RAT] = {
        .name = "Rat",
        .color = {COLOR_LIGHTEST_GRAY},
        .glyph = 'r',

        .hit_die = "1d2",

        .natural_armor_bonus = 1,

        .natural_weapon_type = NATURAL_WEAPON_TYPE_BITE,
    },
    [CLASS_RED_DRAGON_WYRMLING] = {
        .name = "Wyrmling Red Dragon",
        .color = {COLOR_LIGHT_RED},
        .glyph = 'D',

        .hit_die = "1d12",

        .natural_armor_bonus = 6,

        .base_attack_bonus = 7,
    },
    [CLASS_RED_DRAGON_ADULT] = {
        .name = "Adult Red Dragon",
        .color = {COLOR_RED},
        .glyph = 'D',

        .hit_die = "1d12",

        .natural_armor_bonus = 21,

        .base_attack_bonus = 22,
    },
    [CLASS_RED_DRAGON_ANCIENT] = {
        .name = "Ancient Red Dragon",
        .color = {COLOR_DARK_RED},
        .glyph = 'D',

        .hit_die = "1d12",

        .natural_armor_bonus = 33,

        .base_attack_bonus = 34,
    },
    [CLASS_TROLL] = {
        .name = "Rat",
        .color = {COLOR_GREEN},
        .glyph = 'T',

        .hit_die = "1d8",

        .natural_armor_bonus = 5,

        .base_attack_bonus = 4,

        .natural_weapon_type = NATURAL_WEAPON_TYPE_CLAW,
    },
    [CLASS_SKELETON_WARRIOR] = {
        .name = "Skeleton Warrior",
        .color = {COLOR_WHITE},
        .glyph = 's',

        .hit_die = "1d12",
    },
};

const struct feat_data feat_database[] = {
    [FEAT_ARMOR_PROFICIENCY_LIGHT] = {
        .name = "Armor Proficiency: Light",
        .description = "A character with this feat can equip light armor.",
    },
    [FEAT_ARMOR_PROFICIENCY_MEDIUM] = {
        .name = "Armor Proficiency: Medium",
        .description = "A character with this feat can equip medium armor.",
    },
    [FEAT_ARMOR_PROFICIENCY_HEAVY] = {
        .name = "Armor Proficiency: Heavy",
        .description = "A character with this feat can equip heavy armor.",
    },
    [FEAT_QUICK_TO_MASTER] = {
        .name = "Quick to Master",
        .description = "A character with this feat gains an additional feat at level 1.",

        .prerequisites = {
            .requires_race = true,
            .race = RACE_HUMAN,
        },
    },
    [FEAT_RAPID_RELOAD] = {
        .name = "Rapid Reload",
        .description = "A character gets the same number of attacks with any crossbow as a normal bow.",

        .prerequisites = {
            .base_attack_bonus = 2,
        },
    },
    [FEAT_SHIELD_PROFICIENCY] = {
        .name = "Shield Proficiency",
        .description = "A character with this feat can equip shields.",
    },
    [FEAT_SNEAK_ATTACK] = {
        .name = "Sneak Attack",
        .description = "A character with this feat deals extra damage when undetected.",

        .prerequisites = {
            .requires_class = true,
            .class = CLASS_ROGUE,
        },
    },
    [FEAT_STILL_SPELL] = {
        .name = "Still Spell",
        .description = "A character with this feat ignores arcane spell failure.",
    },
    [FEAT_WEAPON_FINESSE] = {
        .name = "Weapon Finesse",
        .description = "A character with this feat is adept at using light weapons subtly and effectively, allowing him to make melee attack rolls with his dexterity modifier instead of strength (if his dexterity is higher than his strength).",

        .prerequisites = {
            .base_attack_bonus = 1,
        },
    },
    [FEAT_WEAPON_PROFICIENCY_ELF] = {
        .name = "Weapon Proficiency: Elf",
        .description = "A character with this feat is proficient with elf weapons.",

        .prerequisites = {
            .requires_race = true,
            .race = RACE_ELF,
        },
    },
    [FEAT_WEAPON_PROFICIENCY_EXOTIC] = {
        .name = "Weapon Proficiency: Exotic",
        .description = "A character with this feat is proficient with exotic weapons.",
    },
    [FEAT_WEAPON_PROFICIENCY_MARTIAL] = {
        .name = "Weapon Proficiency: Martial",
        .description = "A character with this feat is proficient with martial weapons.",
    },
    [FEAT_WEAPON_PROFICIENCY_ROGUE] = {
        .name = "Weapon Proficiency: Rogue",
        .description = "A character with this feat is proficient with rogue weapons.",

        .prerequisites = {
            .requires_class = true,
            .class = CLASS_ROGUE,
        },
    },
    [FEAT_WEAPON_PROFICIENCY_SIMPLE] = {
        .name = "Weapon Proficiency: Simple",
        .description = "A character with this feat is proficient with simple weapons.",
    },
    [FEAT_WEAPON_PROFICIENCY_WIZARD] = {
        .name = "Weapon Proficiency: Wizard",
        .description = "A character with this feat is proficient with wizard weapons.",

        .prerequisites = {
            .requires_class = true,
            .class = CLASS_WIZARD,
        },
    },
};

const struct actor_metadata actor_metadata = {
    .turns_to_chase = 10,
};

struct actor *actor_new(
    const char *const name,
    const enum race race,
    const enum class class,
    const enum faction faction,
    const int level,
    const int ability_scores[NUM_ABILITIES],
    const bool special_abilities[NUM_SPECIAL_ABILITIES],
    const bool feats[NUM_FEATS],
    const int floor,
    const int x,
    const int y)
{
    struct actor *const actor = malloc(sizeof(*actor));

    actor->name = strdup(name);

    actor->race = race;
    actor->class = class;
    actor->faction = faction;

    actor->level = 1;
    actor->experience = actor_calc_experience_for_level(level);

    actor->ability_points = 0;
    for (enum ability ability = 0; ability < NUM_ABILITIES; ability++)
    {
        actor->ability_scores[ability] = ability_scores[ability];
    }

    for (enum feat feat = 0; feat < NUM_FEATS; feat++)
    {
        actor->feats[feat] = feats[feat];
    }

    for (enum special_ability special_ability = 0; special_ability < NUM_SPECIAL_ABILITIES; special_ability++)
    {
        actor->special_abilities[special_ability] = special_abilities[special_ability];
    }

    actor->base_hit_points = TCOD_random_dice_new(class_database[actor->class].hit_die).nb_faces;

    actor->hit_points = actor_calc_max_hit_points(actor);

    actor->gold = 0;

    for (enum equip_slot equip_slot = 0; equip_slot < NUM_EQUIP_SLOTS; equip_slot++)
    {
        actor->equipment[equip_slot] = NULL;
    }

    actor->items = list_new();

    actor->mana = actor_calc_max_mana(actor);
    actor->known_spells = list_new();
    actor->readied_spell = SPELL_TYPE_NONE;

    actor->floor = floor;
    actor->x = x;
    actor->y = y;

    actor->fov = NULL;

    actor->took_turn = false;
    actor->energy = actor_calc_speed(actor);

    actor->current_target = NULL;
    actor->last_seen_x = -1;
    actor->last_seen_y = -1;
    actor->turns_chased = 0;

    actor->leader = NULL;

    actor->light_type = LIGHT_TYPE_NONE;
    actor->light_fov = NULL;

    actor->flash_color = color_white;
    actor->flash_fade_coef = 0;

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

    list_delete(actor->known_spells);

    for (size_t item_index = 0; item_index < actor->items->size; item_index++)
    {
        item_delete(list_get(actor->items, item_index));
    }
    list_delete(actor->items);

    free(actor->name);

    free(actor);
}

int actor_calc_experience_for_level(const int level)
{
    return level * (level - 1) / 2 * 1000;
}

void actor_give_experience(struct actor *const actor, const int experience)
{
    actor->experience += experience;

    world_log(
        actor->floor,
        actor->x,
        actor->y,
        color_azure,
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
            color_yellow,
            "%s has gained a level!",
            actor->name);
    }
}

void actor_level_up(struct actor *const actor)
{
    const float current_hit_point_percent = (float)actor->hit_points / actor_calc_max_hit_points(actor);
    const float current_mana_percent = (float)actor->mana / actor_calc_max_mana(actor);

    actor->level++;
    actor->ability_points++;
    actor->base_hit_points += TCOD_random_dice_roll_s(world->random, class_database[actor->class].hit_die);

    actor->hit_points = (int)(actor_calc_max_hit_points(actor) * current_hit_point_percent);
    actor->mana = (int)(actor_calc_max_mana(actor) * current_mana_percent);

    if (actor->hit_points <= 0)
    {
        actor_die(actor, NULL);
    }
    if (actor->mana <= 0)
    {
        actor->mana = 0;
    }
}

void actor_add_ability_point(struct actor *const actor, const enum ability ability)
{
    const float current_hit_point_percent = (float)actor->hit_points / actor_calc_max_hit_points(actor);
    const float current_mana_percent = (float)actor->mana / actor_calc_max_mana(actor);

    actor->ability_scores[ability]++;
    actor->ability_points--;

    actor->hit_points = (int)(actor_calc_max_hit_points(actor) * current_hit_point_percent);
    actor->mana = (int)(actor_calc_max_mana(actor) * current_mana_percent);

    if (actor->hit_points <= 0)
    {
        actor_die(actor, NULL);
    }
    if (actor->mana <= 0)
    {
        actor->mana = 0;
    }
}

int actor_calc_ability_modifer(const struct actor *const actor, const enum ability ability)
{
    int modifier = (actor->ability_scores[ability] - 10) / 2;

    if (ability == ABILITY_DEXTERITY)
    {
        const struct item *const armor = actor->equipment[EQUIP_SLOT_ARMOR];

        if (armor)
        {
            const struct item_data *const armor_data = &item_database[armor->type];
            const struct base_item_data *const base_armor_data = &base_item_database[armor_data->type];

            if (base_armor_data->max_dexterity_bonus > 0 &&
                modifier > base_armor_data->max_dexterity_bonus)
            {
                modifier = base_armor_data->max_dexterity_bonus;
            }
        }
    }

    return modifier;
}

int actor_calc_max_hit_points(const struct actor *const actor)
{
    return actor->base_hit_points + (actor->level * actor_calc_ability_modifer(actor, ABILITY_CONSTITUTION));
}

void actor_restore_hit_points(struct actor *const actor, const int health)
{
    actor->hit_points += health;
    actor->flash_color = color_green;
    actor->flash_fade_coef = 1;

    const int max_health = actor_calc_max_hit_points(actor);
    if (actor->hit_points > max_health)
    {
        actor->hit_points = max_health;
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
    actor->flash_fade_coef = 1;

    if (actor->hit_points <= 0)
    {
        actor->hit_points = 0;

        actor_die(actor, attacker);

        return true;
    }

    return false;
}

int actor_calc_armor_class(const struct actor *const actor)
{
    const int dexterity_modifer = actor_calc_ability_modifer(actor, ABILITY_DEXTERITY);
    const int natural_armor_bonus = class_database[actor->class].natural_armor_bonus;
    const int size_modifer = size_database[race_database[actor->race].size].modifier;

    int armor_class = 10 + dexterity_modifer + natural_armor_bonus + size_modifer;

    const struct item *const armor = actor->equipment[EQUIP_SLOT_ARMOR];

    if (armor)
    {
        const struct item_data *const armor_data = &item_database[armor->type];
        const struct base_item_data *const base_armor_data = &base_item_database[armor_data->type];

        armor_class += base_armor_data->armor_class;
        armor_class += armor_data->enhancement_bonus;
    }

    const struct item *const shield = actor->equipment[EQUIP_SLOT_SHIELD];

    if (shield)
    {
        const struct item_data *const shield_data = &item_database[shield->type];
        const struct base_item_data *const base_shield_data = &base_item_database[shield_data->type];

        armor_class += base_shield_data->armor_class;
        armor_class += shield_data->enhancement_bonus;
    }

    return armor_class;
}

int actor_calc_attacks_per_round(const struct actor *const actor)
{
    const int base_attack_bonus = actor_calc_base_attack_bonus(actor);

    int attacks_per_round = (base_attack_bonus - 5) / 5;

    const struct item *const weapon = actor->equipment[EQUIP_SLOT_WEAPON];

    if (weapon)
    {
        const struct item_data *const weapon_data = &item_database[weapon->type];
        const struct base_item_data *const base_weapon_data = &base_item_database[weapon_data->type];

        if (base_weapon_data->ranged &&
            (weapon_data->type == BASE_ITEM_TYPE_LIGHT_CROSSBOW ||
             weapon_data->type == BASE_ITEM_TYPE_HEAVY_CROSSBOW) &&
            !actor_has_feat(actor, FEAT_RAPID_RELOAD))
        {
            attacks_per_round = 1;
        }
    }

    // TODO: monk using unarmed or kama
    // return (base_attack_bonus - 3) / 3;

    // TODO: dual wielding? off hand weapon gives extra attacks at BAB -10 / 5

    // TODO: rapid shot feat, gives extra attack per round if ranged (not crossbow)
    // need to implement this in the attack code as well, since all attacks in round suffer -2 penalty

    if (attacks_per_round < 1)
    {
        return 1;
    }

    return attacks_per_round;
}

int actor_calc_base_attack_bonus(const struct actor *const actor)
{
    const enum base_attack_bonus_progression base_attack_bonus_progression = class_database[actor->class].base_attack_bonus_progression;

    if (base_attack_bonus_progression == BASE_ATTACK_BONUS_FIXED)
    {
        return class_database[actor->class].base_attack_bonus;
    }

    return (int)floorf(actor->level * base_attack_bonus_progression_database[base_attack_bonus_progression].multiplier);
}

int actor_calc_attack_bonus(const struct actor *const actor)
{
    const int base_attack_bonus = actor_calc_base_attack_bonus(actor);
    const int size_modifer = size_database[race_database[actor->race].size].modifier;

    int attack_bonus = base_attack_bonus + size_modifer;

    const struct item *const weapon = actor->equipment[EQUIP_SLOT_WEAPON];
    if (weapon)
    {
        const struct item_data *const weapon_data = &item_database[weapon->type];

        attack_bonus += weapon_data->enhancement_bonus;

        const struct base_item_data *const base_weapon_data = &base_item_database[weapon_data->type];

        if (base_weapon_data->ranged)
        {
            attack_bonus += actor_calc_ability_modifer(actor, ABILITY_DEXTERITY);

            const struct item *const ammunition = actor->equipment[EQUIP_SLOT_AMMUNITION];

            if (ammunition)
            {
                const struct item_data *const ammunition_data = &item_database[ammunition->type];

                attack_bonus += ammunition_data->enhancement_bonus;
            }
        }
        else
        {
            const enum equippability equippability = actor_calc_item_equippability(actor, weapon);

            if (actor_has_feat(actor, FEAT_WEAPON_FINESSE) && base_weapon_data->finesse && equippability != EQUIPPABILITY_BARELY)
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
        if (actor_has_feat(actor, FEAT_WEAPON_FINESSE))
        {
            attack_bonus += actor_calc_ability_modifer(actor, ABILITY_DEXTERITY);
        }
        else
        {
            attack_bonus += actor_calc_ability_modifer(actor, ABILITY_STRENGTH);
        }
    }

    return attack_bonus;
}

int actor_calc_ranged_attack_penalty(const struct actor *const actor, const struct actor *const other)
{
    const struct item *const weapon = actor->equipment[EQUIP_SLOT_WEAPON];
    if (weapon)
    {
        const struct item_data *const weapon_data = &item_database[weapon->type];
        const struct base_item_data *const base_weapon_data = &base_item_database[weapon_data->type];

        if (base_weapon_data->ranged &&
            distance_between(actor->x, actor->y, other->x, other->y) < 2 &&
            !actor_has_feat(actor, FEAT_POINT_BLANK_SHOT))
        {
            return 4;
        }
    }

    return 0;
}

int actor_calc_threat_range(const struct actor *const actor)
{
    const struct item *const weapon = actor->equipment[EQUIP_SLOT_WEAPON];

    if (weapon)
    {
        const struct item_data *const weapon_data = &item_database[weapon->type];
        const struct base_item_data *const base_weapon_data = &base_item_database[weapon_data->type];

        return base_weapon_data->threat_range;
    }

    return 20;
}

int actor_calc_critical_multiplier(const struct actor *const actor)
{
    const struct item *const weapon = actor->equipment[EQUIP_SLOT_WEAPON];

    if (weapon)
    {
        const struct item_data *const weapon_data = &item_database[weapon->type];
        const struct base_item_data *const base_weapon_data = &base_item_database[weapon_data->type];

        return base_weapon_data->critical_multiplier;
    }

    return 2;
}

int actor_calc_damage_bonus(const struct actor *const actor)
{
    int damage_bonus = 0;

    const struct item *const weapon = actor->equipment[EQUIP_SLOT_WEAPON];

    if (weapon)
    {
        const struct item_data *const weapon_data = &item_database[weapon->type];

        damage_bonus += weapon_data->enhancement_bonus;

        const struct base_item_data *const base_weapon_data = &base_item_database[weapon_data->type];

        if (base_weapon_data->ranged)
        {
            const int strength_modifier = actor_calc_ability_modifer(actor, ABILITY_STRENGTH);

            if (weapon_data->type == BASE_ITEM_TYPE_SLING)
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
                const struct item_data *const ammunition_data = &item_database[ammunition->type];

                damage_bonus += ammunition_data->enhancement_bonus;
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
        const struct item_data *const weapon_data = &item_database[weapon->type];
        const struct base_item_data *const base_weapon_data = &base_item_database[weapon_data->type];

        return base_weapon_data->damage;
    }

    const struct class_data *const class_data = &class_database[actor->class];

    return natural_weapon_database[class_data->natural_weapon_type].damage;
}

int actor_calc_max_mana(const struct actor *const actor)
{
    // TODO: better formula
    return actor->level * actor_calc_ability_modifer(actor, ABILITY_INTELLIGENCE);
}

void actor_restore_mana(struct actor *const actor, const int mana)
{
    actor->mana += mana;

    const int max_mana = actor_calc_max_mana(actor);
    if (actor->mana > max_mana)
    {
        actor->mana = max_mana;
    }
}

float actor_calc_arcane_spell_failure(const struct actor *const actor)
{
    float arcane_spell_failure = 0;

    if (actor_has_feat(actor, FEAT_STILL_SPELL))
    {
        return 0;
    }

    const struct item *const armor = actor->equipment[EQUIP_SLOT_ARMOR];

    if (armor)
    {
        const struct item_data *const armor_data = &item_database[armor->type];
        const struct base_item_data *const base_armor_data = &base_item_database[armor_data->type];

        arcane_spell_failure += base_armor_data->arcane_spell_failure;
    }

    const struct item *const shield = actor->equipment[EQUIP_SLOT_SHIELD];

    if (shield)
    {
        const struct item_data *const shield_data = &item_database[shield->type];
        const struct base_item_data *const base_shield_data = &base_item_database[shield_data->type];

        arcane_spell_failure += base_shield_data->arcane_spell_failure;
    }

    return arcane_spell_failure;
}

enum equippability actor_calc_item_equippability(const struct actor *const actor, const struct item *const item)
{
    const struct race_data *const race_data = &race_database[actor->race];
    const struct item_data *const item_data = &item_database[item->type];
    const struct base_item_data *const base_item_data = &base_item_database[item_data->type];

    const int difference = race_data->size - base_item_data->size;

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

float actor_calc_max_carry_weight(const struct actor *actor)
{
    return 100 + actor_calc_ability_modifer(actor, ABILITY_STRENGTH) * 10.0f;
}

float actor_calc_carry_weight(const struct actor *actor)
{
    float weight = 0.0f;

    for (size_t item_index = 0; item_index < actor->items->size; item_index++)
    {
        const struct item *const item = list_get(actor->items, item_index);
        const struct item_data *const item_data = &item_database[item->type];
        const struct base_item_data *const base_item_data = &base_item_database[item_data->type];

        weight += base_item_data->weight;
    }

    for (enum equip_slot equip_slot = 1; equip_slot < NUM_EQUIP_SLOTS; equip_slot++)
    {
        const struct item *const item = actor->equipment[equip_slot];

        if (item)
        {
            const struct item_data *const item_data = &item_database[item->type];
            const struct base_item_data *const base_item_data = &base_item_database[item_data->type];

            weight += base_item_data->weight;
        }
    }

    return weight;
}

float actor_calc_speed(const struct actor *actor)
{
    const float speed = size_database[race_database[actor->race].size].speed;
    const bool encumbered = actor_calc_carry_weight(actor) > actor_calc_max_carry_weight(actor);

    return speed * (encumbered ? 0.5f : 1);
}

void actor_calc_light(struct actor *const actor)
{
    if (actor->light_fov)
    {
        TCOD_map_delete(actor->light_fov);
        actor->light_fov = NULL;
    }

    const struct light_data *const light_data = &light_database[actor->light_type];

    if (light_data->radius >= 0)
    {
        actor->light_fov = map_to_fov_map(
            &world->maps[actor->floor],
            actor->x,
            actor->y,
            light_data->radius);
    }
}

void actor_calc_fade(struct actor *const actor, const float delta_time)
{
    if (actor->flash_fade_coef > 0)
    {
        // TODO: slower/faster fade depending on circumstances
        // TODO: different fade functions such as sin()
        actor->flash_fade_coef -= 4 * delta_time;
    }
    else
    {
        actor->flash_fade_coef = 0;
    }
}

int actor_calc_sight_radius(const struct actor *actor)
{
    bool special_abilities[NUM_SPECIAL_ABILITIES] = {false};
    actor_calc_special_abilities(actor, &special_abilities);

    if (special_abilities[SPECIAL_ABILITY_DARKVISION])
    {
        return 2;
    }

    if (special_abilities[SPECIAL_ABILITY_LOW_LIGHT_VISION])
    {
        return 2;
    }

    return 1;
}

void actor_calc_fov(struct actor *const actor)
{
    if (actor->fov)
    {
        TCOD_map_delete(actor->fov);
    }

    struct map *const map = &world->maps[actor->floor];

    actor->fov = map_to_fov_map(map, actor->x, actor->y, actor_calc_sight_radius(actor));

    TCOD_Map *los_map = map_to_fov_map(map, actor->x, actor->y, 0);

    for (int x = 0; x < MAP_WIDTH; x++)
    {
        for (int y = 0; y < MAP_HEIGHT; y++)
        {
            if (!TCOD_map_is_in_fov(actor->fov, x, y) &&
                TCOD_map_is_in_fov(los_map, x, y))
            {
                for (size_t object_index = 0; object_index < map->objects->size; object_index++)
                {
                    const struct object *const object = list_get(map->objects, object_index);

                    if (object->light_fov &&
                        TCOD_map_is_in_fov(object->light_fov, x, y))
                    {
                        TCOD_map_set_in_fov(actor->fov, x, y, true);
                    }
                }

                for (size_t actor_index = 0; actor_index < map->actors->size; actor_index++)
                {
                    const struct actor *const other = list_get(map->actors, actor_index);

                    if (other->light_fov &&
                        TCOD_map_is_in_fov(other->light_fov, x, y))
                    {
                        TCOD_map_set_in_fov(actor->fov, x, y, true);
                    }
                }

                for (size_t projectile_index = 0; projectile_index < map->projectiles->size; projectile_index++)
                {
                    const struct projectile *const projectile = list_get(map->projectiles, projectile_index);

                    if (projectile->light_fov &&
                        TCOD_map_is_in_fov(projectile->light_fov, x, y))
                    {
                        TCOD_map_set_in_fov(actor->fov, x, y, true);
                    }
                }

                for (size_t explosion_index = 0; explosion_index < map->explosions->size; explosion_index++)
                {
                    const struct explosion *const explosion = list_get(map->explosions, explosion_index);

                    if (explosion->fov &&
                        TCOD_map_is_in_fov(explosion->fov, x, y))
                    {
                        TCOD_map_set_in_fov(actor->fov, x, y, true);
                    }
                }

                for (size_t surface_index = 0; surface_index < map->surfaces->size; surface_index++)
                {
                    const struct surface *const surface = list_get(map->surfaces, surface_index);

                    if (surface->light_fov &&
                        TCOD_map_is_in_fov(surface->light_fov, x, y))
                    {
                        TCOD_map_set_in_fov(actor->fov, x, y, true);
                    }
                }
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

void actor_calc_feats(const struct actor *actor, bool (*const feats)[NUM_FEATS])
{
    for (enum feat feat = 0; feat < NUM_FEATS; feat++)
    {
        if (race_database[actor->race].feats[feat])
        {
            (*feats)[feat] = true;
        }

        const int level = class_database[actor->class].feat_progression[feat];

        if (level > 0 && level <= actor->level)
        {
            (*feats)[feat] = true;
        }

        if (actor->feats[feat])
        {
            (*feats)[feat] = true;
        }

        // TODO: feats from equipment
    }
}

bool actor_has_feat(const struct actor *const actor, const enum feat feat)
{
    bool known_feats[NUM_FEATS] = {false};
    actor_calc_feats(actor, &known_feats);

    return known_feats[feat];
}

bool actor_has_prerequisites_for_feat(const struct actor *actor, enum feat feat)
{
    // TODO: if getting the feat from equipment, the actor should still be able to learn it
    if (actor_has_feat(actor, feat))
    {
        return false;
    }

    const struct feat_prerequisites *const prerequisites = &feat_database[feat].prerequisites;

    if (prerequisites->requires_race &&
        prerequisites->race != actor->race)
    {
        return false;
    }

    if (prerequisites->requires_class &&
        prerequisites->class != actor->class)
    {
        return false;
    }

    if (prerequisites->level > actor->level)
    {
        return false;
    }

    if (feat_database[feat].prerequisites.base_attack_bonus > actor_calc_base_attack_bonus(actor))
    {
        return false;
    }

    return true;
}

void actor_calc_special_abilities(const struct actor *const actor, bool (*const special_abilities)[NUM_SPECIAL_ABILITIES])
{
    for (enum special_ability special_ability = 0; special_ability < NUM_SPECIAL_ABILITIES; special_ability++)
    {
        if (race_database[actor->race].special_abilities[special_ability])
        {
            (*special_abilities)[special_ability] = true;
        }

        if (actor->special_abilities[special_ability])
        {
            (*special_abilities)[special_ability] = true;
        }
    }
}

bool actor_has_special_ability(const struct actor *const actor, const enum special_ability special_ability)
{
    bool special_abilities[NUM_SPECIAL_ABILITIES] = {false};
    actor_calc_special_abilities(actor, &special_abilities);

    return special_abilities[special_ability];
}

void actor_calc_known_spells(const struct actor *const actor, bool (*const known_spells)[NUM_SPELL_TYPES])
{
    for (enum spell_type spell_type = SPELL_TYPE_NONE + 1; spell_type < NUM_SPELL_TYPES; spell_type++)
    {
        const int level = class_database[actor->class].spell_progression[spell_type];

        if (level > 0 && level <= actor->level)
        {
            (*known_spells)[spell_type] = true;
        }
    }

    for (size_t spell_index = 0; spell_index < actor->known_spells->size; spell_index++)
    {
        const enum spell_type spell_type = (enum spell_type)(long long)list_get(actor->known_spells, spell_index);

        (*known_spells)[spell_type] = true;
    }
}

bool actor_knows_spell(const struct actor *const actor, const enum spell_type spell_type)
{
    bool known_spells[NUM_SPELL_TYPES] = {false};
    actor_calc_known_spells(actor, &known_spells);

    return known_spells[spell_type];
}

bool actor_can_take_turn(const struct actor *const actor)
{
    return actor->energy >= 1 && !actor->dead;
}

bool actor_is_enemy(const struct actor *const actor, const struct actor *const other)
{
    return TCOD_map_is_in_fov(actor->fov, other->x, other->y) &&
           actor->faction != other->faction;
}

bool actor_is_enemy_nearby(const struct actor *const actor)
{
    const struct map *const map = &world->maps[world->player->floor];

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
    const struct map *const map = &world->maps[world->player->floor];

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

bool actor_has_ranged_weapon(const struct actor *actor)
{
    const struct item *const weapon = actor->equipment[EQUIP_SLOT_WEAPON];

    if (weapon)
    {
        const struct item_data *const item_data = &item_database[weapon->type];
        const struct base_item_data *const base_item_data = &base_item_database[item_data->type];

        if (base_item_data->ranged)
        {
            return true;
        }
    }

    return false;
}

bool actor_is_proficient(const struct actor *const actor, const struct item *const item)
{
    const struct item_data *const item_data = &item_database[item->type];
    const struct base_item_data *const base_item_data = &base_item_database[item_data->type];

    bool feats[NUM_FEATS] = {false};
    actor_calc_feats(actor, &feats);

    if (base_item_data->equip_slot == EQUIP_SLOT_WEAPON)
    {
        bool is_weapon_proficient = false;

        for (enum weapon_proficiency weapon_proficiency = WEAPON_PROFICIENCY_NONE + 1; weapon_proficiency < NUM_WEAPON_PROFICIENCIES; weapon_proficiency++)
        {
            if (base_item_data->weapon_proficiencies[weapon_proficiency])
            {
                if (weapon_proficiency == WEAPON_PROFICIENCY_ELF && feats[FEAT_WEAPON_PROFICIENCY_ELF])
                {
                    is_weapon_proficient = true;

                    break;
                }
                else if (weapon_proficiency == WEAPON_PROFICIENCY_EXOTIC && feats[FEAT_WEAPON_PROFICIENCY_EXOTIC])
                {
                    is_weapon_proficient = true;

                    break;
                }
                else if (weapon_proficiency == WEAPON_PROFICIENCY_MARTIAL && feats[FEAT_WEAPON_PROFICIENCY_MARTIAL])
                {
                    is_weapon_proficient = true;

                    break;
                }
                else if (weapon_proficiency == WEAPON_PROFICIENCY_ROGUE && feats[FEAT_WEAPON_PROFICIENCY_ROGUE])
                {
                    is_weapon_proficient = true;

                    break;
                }
                else if (weapon_proficiency == WEAPON_PROFICIENCY_SIMPLE && feats[FEAT_WEAPON_PROFICIENCY_SIMPLE])
                {
                    is_weapon_proficient = true;

                    break;
                }
                else if (weapon_proficiency == WEAPON_PROFICIENCY_WIZARD && feats[FEAT_WEAPON_PROFICIENCY_WIZARD])
                {
                    is_weapon_proficient = true;

                    break;
                }
            }
        }

        return is_weapon_proficient;
    }
    else if (base_item_data->equip_slot == EQUIP_SLOT_ARMOR ||
             base_item_data->equip_slot == EQUIP_SLOT_SHIELD)
    {
        if (base_item_data->armor_proficiency != ARMOR_PROFICIENCY_NONE)
        {
            if (base_item_data->armor_proficiency == ARMOR_PROFICIENCY_HEAVY && feats[FEAT_ARMOR_PROFICIENCY_HEAVY])
            {
                return true;
            }
            else if (base_item_data->armor_proficiency == ARMOR_PROFICIENCY_LIGHT && feats[FEAT_ARMOR_PROFICIENCY_LIGHT])
            {
                return true;
            }
            else if (base_item_data->armor_proficiency == ARMOR_PROFICIENCY_MEDIUM && feats[FEAT_ARMOR_PROFICIENCY_MEDIUM])
            {
                return true;
            }
            else if (base_item_data->armor_proficiency == ARMOR_PROFICIENCY_SHIELD && feats[FEAT_SHIELD_PROFICIENCY])
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

bool actor_ai(struct actor *const actor)
{
    const struct map *const map = &world->maps[actor->floor];
    const struct tile *const tile = &map->tiles[actor->x][actor->y];

    if (actor->hit_points < actor_calc_max_hit_points(actor) / 2)
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

        // is the actor in melee range?
        if (distance_between(
                actor->x, actor->y,
                nearest_enemy->x, nearest_enemy->y) < 2)
        {
            // does the actor have a weapon?
            if (!weapon)
            {
                // look for a melee weapon to equip
                for (size_t item_index = 0; item_index < actor->items->size; item_index++)
                {
                    struct item *const item = list_get(actor->items, item_index);
                    const struct item_data *const item_data = &item_database[item->type];
                    const struct base_item_data *const base_item_data = &base_item_database[item_data->type];

                    if (base_item_data->equip_slot == EQUIP_SLOT_WEAPON &&
                        !base_item_data->ranged &&
                        actor_is_proficient(actor, item))
                    {
                        if (actor_equip(actor, item))
                        {
                            return true;
                        }
                    }
                }
            }

            // if carrying a ranged weapon, look for other options before shooting
            if (actor_has_ranged_weapon(actor))
            {
                const struct item_data *const weapon_data = &item_database[weapon->type];
                const struct base_item_data *const base_weapon_data = &base_item_database[weapon_data->type];

                // look for a melee weapon to equip
                if (!actor_has_feat(actor, FEAT_POINT_BLANK_SHOT))
                {
                    for (size_t item_index = 0; item_index < actor->items->size; item_index++)
                    {
                        struct item *const item = list_get(actor->items, item_index);
                        const struct item_data *const item_data = &item_database[item->type];
                        const struct base_item_data *const base_item_data = &base_item_database[item_data->type];

                        if (base_item_data->equip_slot == EQUIP_SLOT_WEAPON &&
                            !base_item_data->ranged &&
                            actor_is_proficient(actor, item))
                        {
                            if (actor_equip(actor, item))
                            {
                                return true;
                            }
                        }
                    }
                }

                // did not find melee weapon or has point blank shot feat, so check ammo
                bool has_ammunition = false;

                const struct item *const ammunition = actor->equipment[EQUIP_SLOT_AMMUNITION];

                if (ammunition)
                {
                    // is it the correct ammo?
                    const struct item_data *const ammunition_data = &item_database[ammunition->type];
                    const struct base_item_data *const base_ammunition_data = &base_item_database[ammunition_data->type];

                    if (base_ammunition_data->ammunition_type == base_weapon_data->ammunition_type)
                    {
                        has_ammunition = true;
                    }
                }

                // no ammo or wrong ammo, so look for suitable ammo in inventory and equip it
                if (!has_ammunition)
                {
                    for (size_t item_index = 0; item_index < actor->items->size; item_index++)
                    {
                        struct item *const item = list_get(actor->items, item_index);
                        const struct item_data *const item_data = &item_database[item->type];
                        const struct base_item_data *const base_item_data = &base_item_database[item_data->type];

                        if (!base_weapon_data->ranged &&
                            base_item_data->ammunition_type == base_weapon_data->ammunition_type &&
                            actor_is_proficient(actor, item))
                        {
                            // found suitable ammo, so equip it
                            if (actor_equip(actor, item))
                            {
                                return true;
                            }
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
            if (actor_attack(actor, nearest_enemy, NULL))
            {
                return true;
            }
        }
        else
        {
            // is the actor carrying a ranged weapon?
            if (actor_has_ranged_weapon(actor))
            {
                const struct item_data *const weapon_data = &item_database[weapon->type];
                const struct base_item_data *const base_weapon_data = &base_item_database[weapon_data->type];

                // does the actor have ammunition?
                bool has_ammunition = false;

                // does the weapon require ammunition?
                if (base_weapon_data->ammunition_type == AMMUNITION_TYPE_NONE)
                {
                    has_ammunition = true;
                }
                else
                {
                    // check if ammo is equipped
                    const struct item *const ammunition = actor->equipment[EQUIP_SLOT_AMMUNITION];

                    if (ammunition)
                    {
                        const struct item_data *const ammunition_data = &item_database[ammunition->type];
                        const struct base_item_data *const base_ammunition_data = &base_item_database[ammunition_data->type];

                        if (base_ammunition_data->ammunition_type == base_weapon_data->ammunition_type)
                        {
                            has_ammunition = true;
                        }
                    }
                }

                if (!has_ammunition)
                {
                    // if no ammo, try to find suitable ammo in inventory and equip it
                    for (size_t item_index = 0; item_index < actor->items->size; item_index++)
                    {
                        struct item *const item = list_get(actor->items, item_index);
                        const struct item_data *const item_data = &item_database[item->type];
                        const struct base_item_data *const base_item_data = &base_item_database[item_data->type];

                        if (!base_item_data->ranged &&
                            base_item_data->ammunition_type == base_weapon_data->ammunition_type &&
                            actor_is_proficient(actor, item))
                        {
                            // found suitable ammo, so equip it
                            if (actor_equip(actor, item))
                            {
                                return true;
                            }
                        }
                    }

                    // no ammo, so look for a melee weapon to equip
                    for (size_t item_index = 0; item_index < actor->items->size; item_index++)
                    {
                        struct item *const item = list_get(actor->items, item_index);
                        const struct item_data *const item_data = &item_database[item->type];
                        const struct base_item_data *const base_item_data = &base_item_database[item_data->type];

                        if (base_item_data->equip_slot == EQUIP_SLOT_WEAPON &&
                            !base_item_data->ranged &&
                            actor_is_proficient(actor, item))
                        {
                            if (actor_equip(actor, item))
                            {
                                return true;
                            }
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
                struct item *ranged_weapon = NULL;

                for (size_t item_index = 0; item_index < actor->items->size; item_index++)
                {
                    struct item *const item = list_get(actor->items, item_index);
                    const struct item_data *const item_data = &item_database[item->type];
                    const struct base_item_data *const base_item_data = &base_item_database[item_data->type];

                    if (base_item_data->equip_slot == EQUIP_SLOT_WEAPON &&
                        base_item_data->ranged &&
                        actor_is_proficient(actor, item))
                    {
                        ranged_weapon = item;
                    }
                }

                if (ranged_weapon)
                {
                    const struct item_data *const weapon_data = &item_database[ranged_weapon->type];
                    const struct base_item_data *const base_weapon_data = &base_item_database[weapon_data->type];

                    // does the ranged weapon require ammo?
                    bool should_equip = false;

                    if (base_weapon_data->ammunition_type == AMMUNITION_TYPE_NONE)
                    {
                        should_equip = true;
                    }
                    else
                    {
                        // look for ammo in inventory
                        for (size_t item_index = 0; item_index < actor->items->size; item_index++)
                        {
                            struct item *const item = list_get(actor->items, item_index);
                            const struct item_data *const item_data = &item_database[item->type];
                            const struct base_item_data *const base_item_data = &base_item_database[item_data->type];

                            if (!base_item_data->ranged &&
                                base_item_data->ammunition_type == base_weapon_data->ammunition_type &&
                                actor_is_proficient(actor, item))
                            {
                                should_equip = true;
                            }
                        }
                    }

                    // was ammunition found or not needed?
                    if (should_equip)
                    {
                        // equip the ranged weapon
                        if (actor_equip(actor, ranged_weapon))
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
                for (size_t item_index = 0; item_index < actor->items->size; item_index++)
                {
                    struct item *const item = list_get(actor->items, item_index);
                    const struct item_data *const item_data = &item_database[item->type];
                    const struct base_item_data *const base_item_data = &base_item_database[item_data->type];

                    if (base_item_data->equip_slot == EQUIP_SLOT_WEAPON &&
                        !base_item_data->ranged &&
                        actor_is_proficient(actor, item))
                    {
                        if (actor_equip(actor, item))
                        {
                            return true;
                        }
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
    if (tile->items->size > 0)
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

    actor_restore_hit_points(actor, actor_calc_max_hit_points(actor) - actor->hit_points);
    actor_restore_mana(actor, actor_calc_max_mana(actor) - actor->mana);

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
            // TODO: trap effects

            list_remove(map->objects, tile->object);

            object_delete(tile->object);
            tile->object = NULL;

            world_log(
                actor->floor,
                actor->x,
                actor->y,
                color_white,
                "%s triggers a trap!",
                actor->name);
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
                return actor_attack(actor, tile->actor, NULL);
            }
        }
    }

    struct tile *const current_tile = &map->tiles[actor->x][actor->y];
    current_tile->actor = NULL;

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

    struct map *const map = &world->maps[actor->floor];
    struct tile *const tile = &map->tiles[x][y];

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

    struct map *const map = &world->maps[actor->floor];
    struct tile *const tile = &map->tiles[x][y];

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

    // TODO: prayer effects

    list_remove(map->objects, tile->object);

    object_delete(tile->object);
    tile->object = NULL;

    world_log(
        actor->floor,
        actor->x,
        actor->y,
        color_orange,
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
            color_white,
            "%s can't drink here.",
            actor->name);

        return false;
    }

    actor_restore_hit_points(actor, actor_calc_max_hit_points(actor) - actor->hit_points);
    actor_restore_mana(actor, actor_calc_max_mana(actor) - actor->mana);

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

    struct map *const map = &world->maps[actor->floor];
    struct tile *const tile = &map->tiles[x][y];

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

    if (tile->items->size == 0)
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

    struct item *const item = list_get(tile->items, 0);
    list_remove(map->items, item);
    list_remove(tile->items, item);

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

    // add item to map
    struct map *const map = &world->maps[item->floor];
    list_add(map->items, item);

    // add item to tile
    struct tile *const tile = &map->tiles[item->x][item->y];
    list_add(tile->items, item);

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
    const enum equip_slot equip_slot = base_item_data->equip_slot;

    if (equip_slot == EQUIP_SLOT_NONE)
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
    const enum equippability equippability = actor_calc_item_equippability(actor, item);

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
    list_remove(actor->items, item);

    // add to slot
    actor->equipment[equip_slot] = item;

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
    struct item *equipment = actor->equipment[equip_slot];

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

    if (item_data->type != BASE_ITEM_TYPE_SCROLL &&
        item_data->type != BASE_ITEM_TYPE_TOME)
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

    if (item_data->type == BASE_ITEM_TYPE_SCROLL)
    {
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
    }
    else if (item_data->type == BASE_ITEM_TYPE_TOME)
    {
        // TODO: if already known, do nothing

        // add spell to known spells
        list_add(actor->known_spells, (void *)(size_t)item_data->spell_type);

        // remove from inventory
        list_remove(actor->items, item);

        // delete the item
        item_delete(item);
    }

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
    const struct item_data *const weapon_data = &item_database[weapon->type];
    const struct base_item_data *const base_weapon_data = &base_item_database[weapon_data->type];

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
        const struct item_data *const ammunition_data = &item_database[ammunition->type];
        const struct base_item_data *const base_ammunition_data = &base_item_database[ammunition_data->type];

        if (base_ammunition_data->ammunition_type != base_weapon_data->ammunition_type)
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
        actor->x,
        actor->y,
        x,
        y,
        actor,
        item_new(
            ammunition->type,
            ammunition->floor,
            ammunition->x,
            ammunition->y,
            1));

    // add the projectile to the map
    struct map *const map = &world->maps[actor->floor];
    list_add(map->projectiles, projectile);

    // decrement the actor's ammunition
    ammunition->stack--;

    // unequip and delete the item if out of ammo
    if (ammunition->stack <= 0)
    {
        // remove from slot
        const struct item_data *const ammunition_data = &item_database[ammunition->type];
        const struct base_item_data *const base_ammunition_data = &base_item_database[ammunition_data->type];

        actor->equipment[base_ammunition_data->equip_slot] = NULL;

        // delete the item
        item_delete(ammunition);
    }

    return true;
}

// TODO: rename this function to actor_do_damage and replace all calls with actor_melee, which will do distance checking
bool actor_attack(struct actor *const actor, struct actor *const other, const struct item *const ammunition)
{
    ammunition;

    // calculate other armor class
    const int armor_class = actor_calc_armor_class(other);

    // calculate number of attacks
    const int attacks_per_round = actor_calc_attacks_per_round(actor);

    for (int attack = 0; attack < attacks_per_round; attack++)
    {
        // calculate hit
        const int attack_roll = TCOD_random_dice_roll_s(world->random, "1d20");
        const int attack_bonus = actor_calc_attack_bonus(actor);
        const int successive_attack_penalty = attack * 5;
        const int ranged_attack_penalty = actor_calc_ranged_attack_penalty(actor, other);
        const int hit_challenge = attack_roll + attack_bonus - ranged_attack_penalty - successive_attack_penalty;

        if (attack_roll == 1 ||
            (attack_roll != 20 && hit_challenge < armor_class))
        {
            other->flash_color = color_gray;
            other->flash_fade_coef = 1;

            world_log(
                actor->floor,
                actor->x,
                actor->y,
                color_light_gray,
                "%s misses %s.",
                actor->name,
                other->name);

            return true;
        }

        // calculate critical hit
        bool crit = false;

        if (attack_roll >= actor_calc_threat_range(actor))
        {
            const int threat_roll = TCOD_random_dice_roll_s(world->random, "1d20");
            const int crit_challenge = threat_roll + attack_bonus;

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

        for (size_t i = 0; i < num_attack_rolls; i++)
        {
            const int weapon_damage = TCOD_random_dice_roll_s(world->random, actor_calc_damage(actor));

            damage += weapon_damage + damage_bonus;
        }

        // sneak attack
        bool sneak_attack = false;

        if (actor_has_feat(actor, FEAT_SNEAK_ATTACK) && other->current_target != actor)
        {
            sneak_attack = true;

            for (size_t i = 0; i < actor->level / 2; i++)
            {
                damage += TCOD_random_dice_roll_s(world->random, "1d6");
            }
        }

        // if it's a hit, it has to do damage
        if (damage < 1)
        {
            damage = 1;
        }

        // TODO: when projectiles come at the player from the dark, nothing gets logged
        // it'd be nice if there were a way to do something like "someone attacks <player> for <damage>"
        world_log(
            actor->floor,
            actor->x,
            actor->y,
            crit ? color_light_red : color_white,
            "%s %s %s for %d%s.",
            actor->name,
            crit ? "crits" : "hits",
            other->name,
            damage,
            sneak_attack ? " (sneak)" : "");

        // deal damage
        const bool killed = actor_damage_hit_points(other, actor, damage);

        if (killed)
        {
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

    if (from_memory)
    {
        // does the actor know the spell?
        // this should never happen, but just in case
        // an actor should only be able to "ready" a spell they know
        if (!actor_knows_spell(actor, spell_type))
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
        if (actor->mana < spell_data->level)
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

        // arcane spell failure
        const float arcane_spell_failure = actor_calc_arcane_spell_failure(actor);

        if (arcane_spell_failure > 0)
        {
            const float roll = TCOD_random_get_float(world->random, 0, 1);

            if (roll < arcane_spell_failure)
            {
                world_log(
                    actor->floor,
                    actor->x,
                    actor->y,
                    color_white,
                    "%s's spell fizzles.",
                    actor->name,
                    spell_data->name);

                if (from_memory)
                {
                    actor->mana -= spell_data->level;
                }

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

    if (from_memory)
    {
        actor->mana -= spell_data->level;
    }

    switch (spell_type)
    {
    case SPELL_TYPE_MINOR_HEAL:
    {
        struct map *const map = &world->maps[actor->floor];
        struct tile *const tile = &map->tiles[x][y];
        struct actor *const other = tile->actor;
        if (other)
        {
            const int health = TCOD_random_dice_roll_s(world->random, "1d4") + actor_calc_ability_modifer(actor, ABILITY_INTELLIGENCE);

            world_log(
                other->floor,
                other->x,
                other->y,
                color_white,
                "%s heals for %d.",
                other->name,
                health);

            actor_restore_hit_points(other, health);
        }
    }
    break;
    case SPELL_TYPE_MINOR_MANA:
    {
        struct map *const map = &world->maps[actor->floor];
        struct tile *const tile = &map->tiles[x][y];
        struct actor *const other = tile->actor;
        if (other)
        {
            const int mana = TCOD_random_dice_roll_s(world->random, "1d4") + actor_calc_ability_modifer(actor, ABILITY_INTELLIGENCE);

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
        list_add(map->projectiles, projectile);
    }
    case SPELL_TYPE_ACID_SPLASH:
    {
        if (x == actor->x && y == actor->y)
        {
            return false;
        }

        // create a fireball projectile
        struct projectile *const projectile = projectile_new(
            PROJECTILE_TYPE_ACID_SPLASH,
            actor->floor,
            actor->x,
            actor->y,
            x,
            y,
            actor,
            NULL);

        // add the projectile to the map
        struct map *const map = &world->maps[actor->floor];
        list_add(map->projectiles, projectile);
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
    }
}
