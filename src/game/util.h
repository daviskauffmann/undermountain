#ifndef UM_GAME_UTIL_H
#define UM_GAME_UTIL_H

#include <libtcod.h>

#define PI 3.14f

float distance_between_sq(
    int x1, int y1,
    int x2, int y2);
float distance_between(
    int x1, int y1,
    int x2, int y2);
float angle_between(
    int x1, int y1,
    int x2, int y2);

#endif
