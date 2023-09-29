#ifndef UM_GAME_BASE_ATTACK_BONUS_PROGRESSION_H
#define UM_GAME_BASE_ATTACK_BONUS_PROGRESSION_H

enum base_attack_bonus_progression
{
    BASE_ATTACK_BONUS_FIXED,
    BASE_ATTACK_BONUS_PROGRESSION_COMBAT,
    BASE_ATTACK_BONUS_PROGRESSION_MIDDLE,
    BASE_ATTACK_BONUS_PROGRESSION_NON_COMBAT,

    NUM_BASE_ATTACK_BONUS_PROGRESSIONS
};

struct base_attack_bonus_progression_data
{
    const char *name;
    float multiplier;
};

extern const struct base_attack_bonus_progression_data base_attack_bonus_progression_database[NUM_BASE_ATTACK_BONUS_PROGRESSIONS];

#endif
