#ifndef UM_GAME_SPELL_H
#define UM_GAME_SPELL_H

#include "magic_type.h"
#include "spell_range.h"
#include <libtcod.h>

enum spell_type
{
    SPELL_TYPE_NONE,

    SPELL_TYPE_ACID_SPLASH,
    SPELL_TYPE_CURE_MINOR_WOUNDS,
    SPELL_TYPE_CURE_LIGHT_WOUNDS,
    SPELL_TYPE_CHAIN_LIGHTNING,
    SPELL_TYPE_DAZE,
    SPELL_TYPE_FIREBALL,
    SPELL_TYPE_INFLICT_MINOR_WOUNDS,
    SPELL_TYPE_INFLICT_LIGHT_WOUNDS,
    SPELL_TYPE_MAGIC_MISSILE,
    SPELL_TYPE_RAY_OF_FROST,
    SPELL_TYPE_RECOVER_LIGHT_ARCANA,
    SPELL_TYPE_SUMMON_FAMILIAR,

    NUM_SPELL_TYPES
};

struct spell_data
{
    const char *name;
    int level;
    enum magic_type magic_type;
    enum spell_range range;
    bool harmful;
};

extern const struct spell_data spell_database[NUM_SPELL_TYPES];

#endif
