#ifndef CONFIG_H
#define CONFIG_H

#include <libtcod.h>

#include "world.h"

#define SCREEN_WIDTH 80
#define SCREEN_HEIGHT 50
#define WINDOW_TITLE "Roguelike v0.1"

tileinfo_t tileinfo[NUM_TILETYPES];
actorinfo_t actorinfo[NUM_ACTORTYPES];

void config_init(void);

#endif