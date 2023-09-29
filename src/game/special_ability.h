#ifndef UM_GAME_SPECIAL_ABILITY_H
#define UM_GAME_SPECIAL_ABILITY_H

enum special_ability
{
    SPECIAL_ABILITY_DARKVISION,
    SPECIAL_ABILITY_LOW_LIGHT_VISION,

    NUM_SPECIAL_ABILITIES,
};

struct special_ability_data
{
    const char *name;
    const char *description;
};

extern const struct special_ability_data special_ability_database[NUM_SPECIAL_ABILITIES];

#endif
