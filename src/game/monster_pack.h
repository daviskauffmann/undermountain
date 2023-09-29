#ifndef UM_GAME_MONSTER_PACK_H
#define UM_GAME_MONSTER_PACK_H

#include "monster.h"

enum monster_pack
{
    MONSTER_PACK_BATS,
    MONSTER_PACK_BUGBEARS,
    MONSTER_PACK_DIRE_RAT,
    MONSTER_PACK_HYENAS,
    MONSTER_PACK_GNOLLS,
    MONSTER_PACK_GOBLINS,
    MONSTER_PACK_KOBOLDS,
    MONSTER_PACK_RATS,
    MONSTER_PACK_RED_DRAGON,
    MONSTER_PACK_TROLLS,
    MONSTER_PACK_SKELETONS,

    NUM_MONSTER_PACKS,
};

struct monster_pack_data
{
    int min_floor;
    int max_floor;

    struct
    {
        int min_count;
        int max_count;
    } monsters[NUM_MONSTERS];
};

extern const struct monster_pack_data monster_pack_database[NUM_MONSTER_PACKS];

#endif
