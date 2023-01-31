#ifndef GAME_WORLD_H
#define GAME_WORLD_H

#include "map.h"
#include "spell.h"
#include <libtcod.h>

#define NUM_MAPS 20

struct world
{
    TCOD_Random *random;

    unsigned int time;

    struct list *spawned_unique_item_types;

    struct map maps[NUM_MAPS];

    struct actor *player;

    struct actor *hero;
    bool hero_dead;

    struct list *messages;
};

extern struct world *world;

void world_init(void);
void world_uninit(void);
void world_create(struct actor *hero);
void world_save(const char *filename);
void world_load(const char *filename);
void world_update(float delta_time);
bool world_player_can_take_turn(void);
void world_log(int floor, int x, int y, TCOD_ColorRGB color, const char *fmt, ...);

#endif
