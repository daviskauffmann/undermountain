#include "base_attack_bonus_progression.h"

const struct base_attack_bonus_progression_data base_attack_bonus_progression_database[] = {
    [BASE_ATTACK_BONUS_PROGRESSION_COMBAT] = {
        .name = "Combat",
        .multiplier = 1,
    },
    [BASE_ATTACK_BONUS_PROGRESSION_MIDDLE] = {
        .name = "Middle",
        .multiplier = 0.75f,
    },
    [BASE_ATTACK_BONUS_PROGRESSION_NON_COMBAT] = {
        .name = "Non-Combat",
        .multiplier = 0.5f,
    },
};
