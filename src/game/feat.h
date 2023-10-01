#ifndef UM_GAME_FEAT_H
#define UM_GAME_FEAT_H

#include "ability.h"
#include <libtcod.h>

enum feat
{
    FEAT_NONE,

    FEAT_ALERTNESS,
    FEAT_ARMOR_PROFICIENCY_LIGHT,
    FEAT_ARMOR_PROFICIENCY_MEDIUM,
    FEAT_ARMOR_PROFICIENCY_HEAVY,
    FEAT_COMBAT_CASTING,
    FEAT_DODGE,
    FEAT_IMPROVED_INITIATIVE,
    FEAT_IRON_WILL,
    FEAT_MULTIATTACK,
    FEAT_POINT_BLANK_SHOT,
    FEAT_POWER_ATTACK,
    FEAT_QUICK_TO_MASTER,
    FEAT_RAPID_RELOAD,
    FEAT_SHIELD_PROFICIENCY,
    FEAT_SNEAK_ATTACK,
    FEAT_STEALTHY,
    FEAT_STILL_SPELL,
    FEAT_TOUGHNESS,
    FEAT_TRACK,
    FEAT_WEAPON_FINESSE,
    FEAT_WEAPON_FOCUS, // TODO: split into different weapon types
    FEAT_WEAPON_PROFICIENCY_ELF,
    FEAT_WEAPON_PROFICIENCY_EXOTIC,
    FEAT_WEAPON_PROFICIENCY_MARTIAL,
    FEAT_WEAPON_PROFICIENCY_ROGUE,
    FEAT_WEAPON_PROFICIENCY_SIMPLE,
    FEAT_WEAPON_PROFICIENCY_WIZARD,

    NUM_FEATS,
};

enum race;
enum class;

struct feat_prerequisites
{
    bool requires_race;
    enum race race;

    bool requires_class;
    enum class class;

    int level;

    int base_attack_bonus;

    int ability_scores[NUM_ABILITIES];
};

struct feat_data
{
    const char *name;
    const char *description;

    struct feat_prerequisites prerequisites;
};

extern const struct feat_data feat_database[NUM_FEATS];

#endif
