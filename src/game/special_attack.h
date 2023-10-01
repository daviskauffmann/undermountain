#ifndef UM_GAME_SPECIAL_ATTACK_H
#define UM_GAME_SPECIAL_ATTACK_H

enum special_attack
{
    SPECIAL_ATTACK_NONE,

    SPECIAL_ATTACK_DISEASE,
    SPECIAL_ATTACK_IMPROVED_GRAB,
    SPECIAL_ATTACK_POISON,
    SPECIAL_ATTACK_REND,
    SPECIAL_ATTACK_SCARE,
    SPECIAL_ATTACK_TRIP,
    SPECIAL_ATTACK_WEB,

    NUM_SPECIAL_ATTACKS,
};

struct special_attack_data
{
    const char *name;
};

extern const struct special_attack_data special_attack_database[NUM_SPECIAL_ATTACKS];

#endif
