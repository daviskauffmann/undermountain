#include "spell.h"

const struct spell_data spell_database[] = {
    [SPELL_TYPE_ACID_SPLASH] = {
        .name = "Acid Splash",
        .range = SPELL_RANGE_TOUCH,
        .level = 0,
    },
    [SPELL_TYPE_CURE_LIGHT_WOUNDS] = {
        .name = "Cure Light Wounds",
        .range = SPELL_RANGE_TOUCH,
        .level = 1,
    },
    [SPELL_TYPE_CHAIN_LIGHTNING] = {
        .name = "Lightning",
        .range = SPELL_RANGE_TOUCH,
        .level = 6,
    },
    [SPELL_TYPE_FIREBALL] = {
        .name = "Fireball",
        .range = SPELL_RANGE_TOUCH,
        .level = 3,
    },
    [SPELL_TYPE_MAGIC_MISSILE] = {
        .name = "Magic Missile",
        .range = SPELL_RANGE_TOUCH,
        .level = 1,
    },
    [SPELL_TYPE_RECOVER_LIGHT_ARCANA] = {
        .name = "Recover Light Arcana",
        .range = SPELL_RANGE_TOUCH,
        .level = 1,
    },
    [SPELL_TYPE_SUMMON_FAMILIAR] = {
        .name = "Summon Familiar",
        .range = SPELL_RANGE_TOUCH,
        .level = 1,
    },
};
