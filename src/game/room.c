#include "room.h"

#include <assert.h>
#include <libtcod.h>
#include <malloc.h>

#include "world.h"

void room_init(struct room *room, int x, int y, int w, int h)
{
    room->x = x;
    room->y = y;
    room->w = w;
    room->h = h;
}

void room_reset(struct room *room)
{
}

void room_get_random_pos(struct room *room, int *x, int *y)
{
    *x = TCOD_random_get_int(
        world->random,
        room->x,
        room->x + room->w - 1);
    *y = TCOD_random_get_int(
        world->random,
        room->y,
        room->y + room->h - 1);
}
