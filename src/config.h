#ifndef CONFIG_H
#define CONFIG_H

#include <libtcod.h>

#include "map.h"
#include "actor.h"

#define SCREEN_WIDTH 80
#define SCREEN_HEIGHT 50
#define WINDOW_TITLE "Roguelike v0.1"

extern tileinfo_t tileinfo[NUM_TILETYPES];
extern actorinfo_t actorinfo[NUM_ACTORTYPES];

void config_init(void);

#endif