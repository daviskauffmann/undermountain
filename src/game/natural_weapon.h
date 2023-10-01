#ifndef UM_GAME_NATURAL_WEAPON_TYPE_H
#define UM_GAME_NATURAL_WEAPON_TYPE_H

enum natural_weapon_type
{
    NATURAL_WEAPON_TYPE_UNARMED,

    NATURAL_WEAPON_TYPE_BITE,
    NATURAL_WEAPON_TYPE_CLAW,
    NATURAL_WEAPON_TYPE_HOOF,
    NATURAL_WEAPON_TYPE_TALONS,

    NUM_NATURAL_WEAPON_TYPES,
};

struct natural_weapon_data
{
    const char *name;

    const char *damage;
    enum damage_type damage_type;
    int threat_range;
    int critical_multiplier;
};

extern const struct natural_weapon_data natural_weapon_database[NUM_NATURAL_WEAPON_TYPES];

#endif
