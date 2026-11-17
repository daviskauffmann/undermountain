#ifndef UM_GAME_RACE_H
#define UM_GAME_RACE_H

#include "ability.h"
#include "feat.h"
#include "special_ability.h"
#include <libtcod.h>

enum race
{
    RACE_NONE,

    // player races
    RACE_HUMAN,
    RACE_DWARF,
    RACE_ELF,

    // monster races
    RACE_ANIMAL,
    RACE_GIANT,
    RACE_HUMANOID,
    RACE_MAGICAL_BEAST,
    RACE_OUTSIDER,
    RACE_RED_DRAGON,
    RACE_UNDEAD,
    RACE_VERMIN,

    NUM_RACES,
};
#define PLAYER_RACE_BEGIN RACE_HUMAN
#define PLAYER_RACE_END RACE_ELF
#define NUM_PLAYER_RACES PLAYER_RACE_END + 1
#define MONSTER_RACE_BEGIN RACE_ANIMAL
#define MONSTER_RACE_END RACE_VERMIN
#define NUM_MONSTER_RACES = MONSTER_RACE_END + 1

struct race_data
{
    const char *name;

    enum size size;

    int ability_adjustments[NUM_ABILITIES];

    bool special_abilities[NUM_SPECIAL_ABILITIES];

    bool feats[NUM_FEATS];
};

extern const struct race_data race_database[NUM_RACES];

#endif
