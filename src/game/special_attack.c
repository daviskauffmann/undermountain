#include "special_attack.h"

const struct special_attack_data special_attack_database[NUM_SPECIAL_ATTACKS] = {
    [SPECIAL_ATTACK_DISEASE] = {
        .name = "Disease",
    },
    [SPECIAL_ATTACK_POISON] = {
        .name = "Poison",
    },
};
