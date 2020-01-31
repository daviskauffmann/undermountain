#ifndef GAME_WORLD_H
#define GAME_WORLD_H

#include <libtcod.h>

#include "map.h"
#include "spell.h"

#define NUM_MAPS 60

enum world_state
{
    // The first frame of the world.
    WORLD_STATE_AWAKE,
    // It is the player's move.
    WORLD_STATE_PLAY,
    // Waiting for animations to finish. Actors cannot take their turns in this state.
    WORLD_STATE_WAIT,
    // Player is dead.
    WORLD_STATE_LOSE
};

struct world
{
    enum world_state state;
    unsigned int seed;
    TCOD_random_t random;
    unsigned int time;
    int current_actor_index;
    struct actor *player;
    struct map maps[NUM_MAPS];
    TCOD_list_t messages;
    struct actor *hero;
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
