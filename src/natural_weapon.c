#include "natural_weapon.h"

#include "damage_type.h"

const struct natural_weapon_data natural_weapon_database[] = {
    [NATURAL_WEAPON_TYPE_UNARMED] = {
        .name = "Unarmed",

        .damage = "1d3",
        .damage_type = DAMAGE_TYPE_BLUDGEONING,
        .threat_range = 20,
        .critical_multiplier = 2,
    },
    [NATURAL_WEAPON_TYPE_BITE] = {
        .name = "Bite",

        .damage = "1d4",
        .damage_type = DAMAGE_TYPE_PIERCING,
        .threat_range = 20,
        .critical_multiplier = 2,
    },
    [NATURAL_WEAPON_TYPE_CLAW] = {
        .name = "Claw",

        .damage = "1d6",
        .damage_type = DAMAGE_TYPE_SLASHING,
        .threat_range = 20,
        .critical_multiplier = 2,
    },
    [NATURAL_WEAPON_TYPE_HOOF] = {
        .name = "Hoof",

        .damage = "1d4",
        .damage_type = DAMAGE_TYPE_BLUDGEONING,
        .threat_range = 20,
        .critical_multiplier = 2,
    },
    [NATURAL_WEAPON_TYPE_TALONS] = {
        .name = "Talons",

        .damage = "1d4",
        .damage_type = DAMAGE_TYPE_SLASHING,
        .threat_range = 20,
        .critical_multiplier = 2,
    },
};
