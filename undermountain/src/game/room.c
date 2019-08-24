#include "room.h"

#include <assert.h>
#include <malloc.h>
#include <stdio.h>
#include <stdlib.h>

#include <libtcod.h>

struct room *room_new(int x, int y, int w, int h)
{
    struct room *room = malloc(sizeof(struct room));
    assert(room);
    room->x = x;
    room->y = y;
    room->w = w;
    room->h = h;
    return room;
}

void room_delete(struct room *room)
{
    free(room);
}

void room_get_random_pos(struct room *room, int *x, int *y)
{
    *x = TCOD_random_get_int(
        NULL,
        room->x,
        room->x + room->w - 1);
    *y = TCOD_random_get_int(
        NULL,
        room->y,
        room->y + room->h - 1);
}
