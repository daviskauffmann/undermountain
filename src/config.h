#ifndef CONFIG_H
#define CONFIG_H

#include <libtcod.h>

#include "world.h"

#define SCREEN_WIDTH 40
#define SCREEN_HEIGHT 25
#define WINDOW_TITLE "Roguelike v0.1"
#define SFX true

tile_info_t tile_info[NUM_TILETYPES];
actor_info_t actor_info[NUM_ACTORTYPES];

void config_init(void);

#endif