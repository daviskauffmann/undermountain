#include <libtcod.h>

#include "CMemLeak.h"
#include "game.h"

/* Rooms */
room_t *room_create(int x, int y, int w, int h)
{
    room_t *room = (room_t *)malloc(sizeof(room_t));

    room->x = x;
    room->y = y;
    room->w = w;
    room->h = h;

    return room;
}

void room_get_random_pos(room_t *room, int *x, int *y)
{
    *x = TCOD_random_get_int(NULL, room->x, room->x + room->w - 1);
    *y = TCOD_random_get_int(NULL, room->y, room->y + room->h - 1);
}

void room_destroy(room_t *room)
{
    free(room);
}