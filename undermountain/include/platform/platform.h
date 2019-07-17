#ifndef PLATFORM_H
#define PLATFORM_H

#include <libtcod/libtcod.h>

#include <game/game.h>

#include "config.h"
#include "scene.h"
#include "sys.h"
#include "scenes/scene_about.h"
#include "scenes/scene_game.h"
#include "scenes/scene_menu.h"

int platform_run(void);

#endif
