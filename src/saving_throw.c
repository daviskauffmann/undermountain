#include "saving_throw.h"

#include "ability.h"

const struct saving_throw_data saving_throw_database[] = {
    [SAVING_THROW_FORTITUDE] = {
        .name = "Fortitude",

        .ability = ABILITY_CONSTITUTION,
    },
    [SAVING_THROW_REFLEX] = {
        .name = "Reflex",

        .ability = ABILITY_DEXTERITY,
    },
    [SAVING_THROW_WILL] = {
        .name = "Will",

        .ability = ABILITY_WISDOM,
    },
};
