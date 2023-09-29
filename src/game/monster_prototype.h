#ifndef UM_GAME_MONSTER_PROTOTYPE_H
#define UM_GAME_MONSTER_PROTOTYPE_H

#include "ability.h"
#include "actor.h"
#include "item.h"
#include "monster.h"
#include "special_ability.h"

// TODO: maybe merge with monster.h?
// monster -> monster_type
// monster_prototype -> monster_data

struct monster_prototype
{
    const char *name;

    enum race race;
    enum class class;
    enum faction faction;

    int level;

    int ability_scores[NUM_ABILITIES];

    bool special_abilities[NUM_SPECIAL_ABILITIES];

    bool feats[NUM_FEATS];

    struct
    {
        enum item_type type;

        int min_stack;
        int max_stack;
    } equipment[NUM_EQUIP_SLOTS];

    struct
    {
        int min_stack;
        int max_stack;
    } items[NUM_ITEM_TYPES];

    // TODO: spells
};

extern const struct monster_prototype monster_prototypes[NUM_MONSTERS];

#endif
