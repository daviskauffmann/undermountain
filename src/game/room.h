#ifndef UM_GAME_ROOM_H
#define UM_GAME_ROOM_H

#include <stdint.h>

struct room
{
    int x;
    int y;
    int w;
    int h;
};

struct room *room_new(
    int x,
    int y,
    int w,
    int h);
void room_delete(struct room *room);
void room_get_random_pos(
    const struct room *room,
    int *x, int *y);

#endif
