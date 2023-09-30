#ifndef UM_GAME_BASE_ATTACK_BONUS_PROGRESSION_H
#define UM_GAME_BASE_ATTACK_BONUS_PROGRESSION_H

enum base_attack_bonus_type
{
    BASE_ATTACK_BONUS_TYPE_FIXED,
    BASE_ATTACK_BONUS_TYPE_GOOD,
    BASE_ATTACK_BONUS_TYPE_AVERAGE,
    BASE_ATTACK_BONUS_TYPE_POOR,

    NUM_BASE_ATTACK_BONUS_TYPES
};

struct base_attack_bonus_data
{
    const char *name;
    float multiplier;
};

extern const struct base_attack_bonus_data base_attack_bonus_database[NUM_BASE_ATTACK_BONUS_TYPES];

#endif
