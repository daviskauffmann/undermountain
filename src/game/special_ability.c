#include "special_ability.h"

const struct special_ability_data special_ability_database[] = {
    [SPECIAL_ABILITY_DARKVISION] = {
        .name = "Darkvision",
        .description = "Grants the ability to see in the dark.",
    },
    [SPECIAL_ABILITY_LOW_LIGHT_VISION] = {
        .name = "Low-light Vision",
        .description = "Grants the ability to see in the dark, but not as far as darkvision.",
    },
};
