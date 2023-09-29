#include "natural_weapon.h"

const struct natural_weapon_data natural_weapon_database[] = {
    [NATURAL_WEAPON_TYPE_UNARMED] = {
        .name = "Unarmed",

        .damage = "1d3",
        .threat_range = 20,
        .critical_multiplier = 2,
    },
    [NATURAL_WEAPON_TYPE_BITE] = {
        .name = "Bite",

        .damage = "1d4",
        .threat_range = 20,
        .critical_multiplier = 2,
    },
    [NATURAL_WEAPON_TYPE_CLAW] = {
        .name = "Claw",

        .damage = "1d6",
        .threat_range = 20,
        .critical_multiplier = 2,
    },
};
