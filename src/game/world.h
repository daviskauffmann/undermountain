#ifndef GAME_WORLD_H
#define GAME_WORLD_H

#include <libtcod.h>

#include "map.h"
#include "spell.h"

#define NUM_MAPS 60

enum world_state
{
    WORLD_STATE_PLAY,
    WORLD_STATE_WAIT,
    WORLD_STATE_LOSE
};

struct world
{
    enum world_state state;
    unsigned int seed;
    TCOD_random_t random;
    unsigned int turn;
    struct map maps[NUM_MAPS];
    TCOD_list_t messages;
    struct actor *player;
};

extern struct world *world;

void world_init(void);
void world_quit(void);
void world_create(void);
void world_save(const char *filename);
void world_load(const char *filename);
void world_update(float delta_time);
void world_turn(void);
void world_log(int floor, int x, int y, TCOD_color_t color, char *fmt, ...);

#endif
