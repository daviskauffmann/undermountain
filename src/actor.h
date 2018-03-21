#ifndef ACTOR_H
#define ACTOR_H

#include <libtcod/libtcod.h>

struct actor_common
{
    TCOD_color_t torch_color;
    int torch_radius;
};

enum actor_type
{
    ACTOR_PLAYER,
    ACTOR_DOG,
    ACTOR_ORC,
    ACTOR_BUGBEAR,
    ACTOR_JACKAL,
    ACTOR_ZOMBIE,

    NUM_ACTOR_TYPES
};

struct actor_info
{
    const char *name;
    unsigned char glyph;
    TCOD_color_t color;
};

enum faction
{
    FACTION_GOOD,
    FACTION_NEUTRAL,
    FACTION_EVIL
};

struct actor
{
    enum actor_type type;
    struct map *map;
    int x;
    int y;
    int health;
    TCOD_map_t fov;
    TCOD_map_t torch_fov;
    enum faction faction;
    TCOD_list_t items;
};

struct actor *actor_create(enum actor_type type, struct map *map, int x, int y, int health, enum faction faction);
void actor_destroy(struct actor *actor);

#endif
