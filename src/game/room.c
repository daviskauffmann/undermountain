#include "room.h"

#include "world.h"
#include <libtcod.h>
#include <malloc.h>

struct room *room_new(
    const int x,
    const int y,
    const int w,
    const int h)
{
    struct room *const room = malloc(sizeof(*room));

    room->x = x;
    room->y = y;
    room->w = w;
    room->h = h;

    return room;
}

void room_delete(struct room *const room)
{
    free(room);
}

void room_get_random_pos(
    const struct room *const room,
    int *const x, int *const y)
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
