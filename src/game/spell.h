#ifndef UM_GAME_SPELL_H
#define UM_GAME_SPELL_H

#include "spell_range.h"

enum spell_type
{
    SPELL_TYPE_NONE, // TODO: maybe remove this?

    SPELL_TYPE_ACID_SPLASH,
    SPELL_TYPE_CURE_LIGHT_WOUNDS,
    SPELL_TYPE_CHAIN_LIGHTNING,
    SPELL_TYPE_FIREBALL,
    SPELL_TYPE_MAGIC_MISSILE,
    SPELL_TYPE_RECOVER_LIGHT_ARCANA,
    SPELL_TYPE_SUMMON_FAMILIAR,

    NUM_SPELL_TYPES
};

struct spell_data
{
    const char *name;
    enum spell_range range;
    int level;
};

extern const struct spell_data spell_database[NUM_SPELL_TYPES];

#endif
