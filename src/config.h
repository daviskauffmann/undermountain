#ifndef CONFIG_H
#define CONFIG_H

#include "world.h"

#define SCREEN_WIDTH 80
#define SCREEN_HEIGHT 50
#define WINDOW_TITLE "Roguelike v0.1"

extern tileinfo_t tileinfo[NB_TILETYPES];

void config_init(void);

#endif