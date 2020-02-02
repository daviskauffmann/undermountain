#ifndef GAME_WORLD_H
#define GAME_WORLD_H

#include <libtcod.h>

#include "map.h"
#include "spell.h"

#define NUM_MAPS 1

struct world
{
    unsigned int seed;
    TCOD_random_t random;
    unsigned int time;
    struct map maps[NUM_MAPS];
    int current_actor_index;
    struct actor *player;
    struct actor *hero;
    TCOD_list_t messages;
};

extern struct world *world;

void world_init(void);
void world_quit(void);
void world_create(void);
void world_save(const char *filename);
void world_load(const char *filename);
void world_update(float delta_time);
void world_log(int floor, int x, int y, TCOD_color_t color, char *fmt, ...);

#endif
