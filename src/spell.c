#include "spell.h"

#include "magic_type.h"
#include "spell_range.h"

const struct spell_data spell_database[] = {
    [SPELL_TYPE_ACID_SPLASH] = {
        .name = "Acid Splash",
        .level = 0,
        .magic_type = MAGIC_TYPE_ARCANE,
        .range = SPELL_RANGE_TOUCH,
        .harmful = true,
    },
    [SPELL_TYPE_CURE_MINOR_WOUNDS] = {
        .name = "Cure Minor Wounds",
        .level = 0,
        .magic_type = MAGIC_TYPE_DIVINE,
        .range = SPELL_RANGE_TOUCH,
    },
    [SPELL_TYPE_CURE_LIGHT_WOUNDS] = {
        .name = "Cure Light Wounds",
        .level = 1,
        .magic_type = MAGIC_TYPE_DIVINE,
        .range = SPELL_RANGE_TOUCH,
    },
    [SPELL_TYPE_CURE_MODERATE_WOUNDS] = {
        .name = "Cure Moderate Wounds",
        .level = 2,
        .magic_type = MAGIC_TYPE_DIVINE,
        .range = SPELL_RANGE_TOUCH,
    },
    [SPELL_TYPE_CURE_SERIOUS_WOUNDS] = {
        .name = "Cure Serious Wounds",
        .level = 3,
        .magic_type = MAGIC_TYPE_DIVINE,
        .range = SPELL_RANGE_TOUCH,
    },
    [SPELL_TYPE_CURE_CRITICAL_WOUNDS] = {
        .name = "Cure Critical Wounds",
        .level = 4,
        .magic_type = MAGIC_TYPE_DIVINE,
        .range = SPELL_RANGE_TOUCH,
    },
    [SPELL_TYPE_CHAIN_LIGHTNING] = {
        .name = "Lightning",
        .level = 6,
        .magic_type = MAGIC_TYPE_ARCANE,
        .range = SPELL_RANGE_TOUCH,
        .harmful = true,
    },
    [SPELL_TYPE_DAZE] = {
        .name = "Daze",
        .level = 0,
        .magic_type = MAGIC_TYPE_ARCANE,
        .range = SPELL_RANGE_TOUCH,
        .harmful = true,
    },
    [SPELL_TYPE_FIREBALL] = {
        .name = "Fireball",
        .level = 3,
        .magic_type = MAGIC_TYPE_ARCANE,
        .range = SPELL_RANGE_TOUCH,
        .harmful = true,
    },
    [SPELL_TYPE_HARM] = {
        .name = "Harm",
        .level = 6,
        .magic_type = MAGIC_TYPE_DIVINE,
        .range = SPELL_RANGE_TOUCH,
        .harmful = true,
    },
    [SPELL_TYPE_HEAL] = {
        .name = "Heal",
        .level = 6,
        .magic_type = MAGIC_TYPE_DIVINE,
        .range = SPELL_RANGE_TOUCH,
    },
    [SPELL_TYPE_INFLICT_MINOR_WOUNDS] = {
        .name = "Inflict Minor Wounds",
        .level = 0,
        .magic_type = MAGIC_TYPE_DIVINE,
        .range = SPELL_RANGE_TOUCH,
        .harmful = true,
    },
    [SPELL_TYPE_INFLICT_LIGHT_WOUNDS] = {
        .name = "Inflict Light Wounds",
        .level = 1,
        .magic_type = MAGIC_TYPE_DIVINE,
        .range = SPELL_RANGE_TOUCH,
        .harmful = true,
    },
    [SPELL_TYPE_INFLICT_MODERATE_WOUNDS] = {
        .name = "Inflict Moderate Wounds",
        .level = 2,
        .magic_type = MAGIC_TYPE_DIVINE,
        .range = SPELL_RANGE_TOUCH,
        .harmful = true,
    },
    [SPELL_TYPE_INFLICT_SERIOUS_WOUNDS] = {
        .name = "Inflict Serious Wounds",
        .level = 3,
        .magic_type = MAGIC_TYPE_DIVINE,
        .range = SPELL_RANGE_TOUCH,
        .harmful = true,
    },
    [SPELL_TYPE_INFLICT_CRITICAL_WOUNDS] = {
        .name = "Inflict Critical Wounds",
        .level = 4,
        .magic_type = MAGIC_TYPE_DIVINE,
        .range = SPELL_RANGE_TOUCH,
        .harmful = true,
    },
    [SPELL_TYPE_MAGIC_MISSILE] = {
        .name = "Magic Missile",
        .level = 1,
        .magic_type = MAGIC_TYPE_ARCANE,
        .range = SPELL_RANGE_TOUCH,
        .harmful = true,
    },
    [SPELL_TYPE_RAY_OF_FROST] = {
        .name = "Ray of Frost",
        .level = 0,
        .magic_type = MAGIC_TYPE_ARCANE,
        .range = SPELL_RANGE_TOUCH,
        .harmful = true,
    },
    [SPELL_TYPE_RESTORE_MANA] = {
        .name = "Restore Mana",
        .level = 0,
        .magic_type = MAGIC_TYPE_DIVINE,
        .range = SPELL_RANGE_TOUCH,
    },
    [SPELL_TYPE_SUMMON_FAMILIAR] = {
        .name = "Summon Familiar",
        .level = 1,
        .magic_type = MAGIC_TYPE_ARCANE,
        .range = SPELL_RANGE_TOUCH,
    },
};
