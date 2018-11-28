#ifndef ROGUELIKE_H
#define ROGUELIKE_H

#include <float.h>
#include <malloc.h>
#include <math.h>
#include <stdarg.h>
// #include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include <libtcod/libtcod.h> // TODO: this is only here during the transition, eventually the game should be totally platform independent and not include this header
#include <platform/platform.h>

#include "actor.h"
#include "assets.h"
#include "game.h"
#include "item.h"
#include "map.h"
#include "message.h"
#include "object.h"
#include "projectile.h"
#include "room.h"
#include "spell.h"
#include "tile.h"
#include "util.h"

#endif
