#include "race.h"

#include "feat.h"
#include "size.h"

const struct race_data race_database[] = {
    // player races
    [RACE_DWARF] = {
        .name = "Dwarf",

        .size = SIZE_SMALL,

        .ability_adjustments = {
            [ABILITY_CONSTITUTION] = 2,
            [ABILITY_CHARISMA] = -2,
        },

        .special_abilities = {
            [SPECIAL_ABILITY_DARKVISION] = true,
        },
    },
    [RACE_ELF] = {
        .name = "Elf",

        .size = SIZE_MEDIUM,

        .ability_adjustments = {
            [ABILITY_DEXTERITY] = 2,
            [ABILITY_CONSTITUTION] = -2,
        },

        .special_abilities = {
            [SPECIAL_ABILITY_LOW_LIGHT_VISION] = true,
            [SPECIAL_ABILITY_SLEEP_IMMUNITY] = true,
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
    [RACE_ANIMAL] = {
        .name = "Animal",
    },
    [RACE_GIANT] = {
        .name = "Giant",
    },
    [RACE_HUMANOID] = {
        .name = "Humanoid",
    },
    [RACE_MAGICAL_BEAST] = {
        .name = "Magical Beast",
    },
    [RACE_OUTSIDER] = {
        .name = "Outsider",
    },
    [RACE_RED_DRAGON] = {
        .name = "Red Dragon",
    },
    [RACE_UNDEAD] = {
        .name = "Undead",

        .special_abilities = {
            [SPECIAL_ABILITY_DARKVISION] = true,
            // TODO: damage reduction
            // TODO: immunities
        },
    },
    [RACE_VERMIN] = {
        .name = "Small Vermin",

        .special_abilities = {
            [SPECIAL_ABILITY_DARKVISION] = true,
        },
    },
};
