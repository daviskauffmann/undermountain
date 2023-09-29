#ifndef UM_GAME_MONSTER_PACK_H
#define UM_GAME_MONSTER_PACK_H

#include "monster.h"

enum monster_pack_type
{
    MONSTER_PACK_TYPE_BATS,
    MONSTER_PACK_TYPE_BUGBEARS,
    MONSTER_PACK_TYPE_DIRE_RAT,
    MONSTER_PACK_TYPE_FIRE_BEETLES,
    MONSTER_PACK_TYPE_HYENAS,
    MONSTER_PACK_TYPE_GIANT_ANTS,
    MONSTER_PACK_TYPE_GNOLLS,
    MONSTER_PACK_TYPE_GOBLINS,
    MONSTER_PACK_TYPE_KOBOLDS,
    MONSTER_PACK_TYPE_KRENSHARS,
    MONSTER_PACK_TYPE_RATS,
    MONSTER_PACK_TYPE_RED_DRAGON,
    MONSTER_PACK_TYPE_TROLLS,
    MONSTER_PACK_TYPE_SKELETONS,
    MONSTER_PACK_TYPE_SNAKE,
    MONSTER_PACK_TYPE_SPIDERS,
    MONSTER_PACK_TYPE_WOLVES,

    NUM_MONSTER_PACK_TYPES,
};

struct monster_pack_data
{
    int min_floor;
    int max_floor;

    struct
    {
        int min_count;
        int max_count;
    } monsters[NUM_MONSTER_TYPES];
};

extern const struct monster_pack_data monster_pack_database[NUM_MONSTER_PACK_TYPES];

#endif
