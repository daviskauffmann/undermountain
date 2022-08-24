#ifndef UTIL_H
#define UTIL_H

#include <libtcod.h>

#define PI 3.14f

#define TCOD_LIST_FOREACH(list) for (void **iterator = TCOD_list_begin(list); iterator != NULL && iterator != TCOD_list_end(list); iterator++)

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
