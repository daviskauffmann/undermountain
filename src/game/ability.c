#include "ability.h"

const struct ability_data ability_database[] = {
    [ABILITY_STRENGTH] = {
        .name = "Strength",
        .description = "Strength measures the muscle and physical power of a character.",
    },
    [ABILITY_DEXTERITY] = {
        .name = "Dexterity",
        .description = "Dexterity measures agility, reflexes, and balance.",
    },
    [ABILITY_CONSTITUTION] = {
        .name = "Constitution",
        .description = "Constitution represents the health and stamina of a character.",
    },
    [ABILITY_INTELLIGENCE] = {
        .name = "Intelligence",
        .description = "Intelligence determines how well a character learns and reasons.",
    },
};
