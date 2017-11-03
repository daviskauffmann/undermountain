#ifndef GAME_H
#define GAME_H

#include <libtcod.h>

#include "actor.h"

int current_map_index;
actor_t *player;

void game_initialize(void);
void game_save(void);
void game_load(void);
void game_finalize(void);

#endif