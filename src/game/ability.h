#ifndef UM_GAME_ABILITY_H
#define UM_GAME_ABILITY_H

enum ability
{
    ABILITY_STRENGTH,
    ABILITY_DEXTERITY,
    ABILITY_CONSTITUTION,
    ABILITY_INTELLIGENCE,

    NUM_ABILITIES,
};

struct ability_data
{
    const char *name;
    const char *description;
};

extern const struct ability_data ability_database[NUM_ABILITIES];

#endif
