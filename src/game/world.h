#ifndef UM_GAME_WORLD_H
#define UM_GAME_WORLD_H

#include "map.h"
#include <libtcod.h>

#define NUM_MAPS 40

struct world
{
    size_t time;

    struct list *spawned_unique_item_types;

    struct map maps[NUM_MAPS];

    struct actor *player;

    struct actor *hero;
    bool doomed;

    struct list *messages;
};

extern struct world *world;

void world_init(void);
void world_uninit(void);

void world_create(struct actor *hero, unsigned int seed);

void world_save(FILE *file);
void world_load(FILE *file);

void world_update(float delta_time);

bool world_can_player_take_turn(void);

void world_log(int floor, int x, int y, TCOD_ColorRGB color, const char *fmt, ...);

#endif
