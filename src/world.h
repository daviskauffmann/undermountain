#ifndef WORLD_H
#define WORLD_H

#include <stdint.h>
#include <libtcod.h>

#include "map.h"

extern tileinfo_t tileinfo[NB_TILETYPES];
extern TCOD_list_t maps;
extern uint8_t current_map_index;
extern map_t *current_map;
extern actor_t *player;

void world_init(void);
void world_save(void);
void world_load(void);
void world_destroy(void);

#endif