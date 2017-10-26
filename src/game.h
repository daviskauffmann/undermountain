#ifndef GAME_H
#define GAME_H

#include <libtcod.h>

#include "world.h"

int current_map_index;
map_t *current_map;
actor_t *player;

void game_init(void);
void game_load(void);
void game_save(void);
void game_update(void);

#endif