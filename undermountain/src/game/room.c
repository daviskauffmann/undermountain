#include "room.h"

#include <malloc.h>
#include <stdio.h>
#include <stdlib.h>

#include <libtcod.h>

struct room *room_create(int x, int y, int w, int h)
{
    struct room *room = malloc(sizeof(struct room));

    if (!room)
    {
        printf("Couldn't allocate room\n");

        return NULL;
    }

    room->x = x;
    room->y = y;
    room->w = w;
    room->h = h;

    return room;
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

void room_destroy(struct room *room)
{
    free(room);
}
