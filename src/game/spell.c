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
    [SPELL_TYPE_RECOVER_LIGHT_ARCANA] = {
        .name = "Recover Light Arcana",
        .level = 1,
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
