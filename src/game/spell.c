#include "spell.h"

const struct spell_data spell_database[] = {
    [SPELL_TYPE_ACID_SPLASH] = {
        .name = "Acid Splash",
        .range = SPELL_RANGE_TOUCH,
        .level = 1,
    },
    [SPELL_TYPE_MINOR_HEAL] = {
        .name = "Minor Heal",
        .range = SPELL_RANGE_TOUCH,
        .level = 1,
    },
    [SPELL_TYPE_MINOR_MANA] = {
        .name = "Minor Mana",
        .range = SPELL_RANGE_TOUCH,
        .level = 1,
    },
    [SPELL_TYPE_LIGHTNING] = {
        .name = "Lightning",
        .range = SPELL_RANGE_TOUCH,
        .level = 1,
    },
    [SPELL_TYPE_FIREBALL] = {
        .name = "Fireball",
        .range = SPELL_RANGE_TOUCH,
        .level = 1,
    },
    [SPELL_TYPE_SUMMON_FAMILIAR] = {
        .name = "Summon Familiar",
        .range = SPELL_RANGE_TOUCH,
        .level = 1,
    },
};
