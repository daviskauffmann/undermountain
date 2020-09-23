#include "room.h"

#include <assert.h>
#include <libtcod.h>
#include <malloc.h>

#include "world.h"

struct room *room_new(int x, int y, int w, int h)
{
    struct room *room = malloc(sizeof(*room));
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
        world->random,
        room->x,
        room->x + room->w - 1);
    *y = TCOD_random_get_int(
        world->random,
        room->y,
        room->y + room->h - 1);
}
