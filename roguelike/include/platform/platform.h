#ifndef PLATFORM_H
#define PLATFORM_H

#include <libtcod/libtcod.h>
#include <roguelike/roguelike.h>

#include "config.h"
#include "state.h"
#include "state_about.h"
#include "state_game.h"
#include "state_menu.h"
#include "tooltip_option.h"

int platform_run(void);
void platform_set_state(struct state *state);
bool file_exists(const char *filename);

#endif
