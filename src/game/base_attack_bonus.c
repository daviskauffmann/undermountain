#include "base_attack_bonus.h"

const struct base_attack_bonus_data base_attack_bonus_database[] = {
    [BASE_ATTACK_BONUS_TYPE_GOOD] = {
        .name = "Good",
        .multiplier = 1,
    },
    [BASE_ATTACK_BONUS_TYPE_AVERAGE] = {
        .name = "Average",
        .multiplier = 0.75f,
    },
    [BASE_ATTACK_BONUS_TYPE_POOR] = {
        .name = "Poor",
        .multiplier = 0.5f,
    },
};
