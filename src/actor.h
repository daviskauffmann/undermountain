#ifndef ACTOR_H
#define ACTOR_H

#include <libtcod/libtcod.h>

typedef struct
{
    TCOD_color_t torch_color;
    int torch_radius;
} ActorCommon;

typedef enum {
    ACTOR_PLAYER,
    ACTOR_DOG,
    ACTOR_ORC,
    ACTOR_BUGBEAR,
    ACTOR_JACKAL,
    ACTOR_ZOMBIE,

    NUM_ACTOR_TYPES
} ActorType;

typedef struct
{
    const char *name;
    unsigned char glyph;
    TCOD_color_t color;
} ActorInfo;

typedef enum {
    FACTION_GOOD,
    FACTION_NEUTRAL,
    FACTION_EVIL
} Faction;

typedef struct
{
    int x;
    int y;
    int health;
    TCOD_map_t fov;
    TCOD_map_t torch_fov;
    Faction faction;
} Actor;

#endif
