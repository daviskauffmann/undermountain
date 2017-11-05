#include <stdlib.h>
#include <libtcod.h>

#include "CMemLeak.h"
#include "game.h"

room_t *room_create(map_t *map, int x, int y, int w, int h)
{
    room_t *room = (room_t *)malloc(sizeof(room_t));

    room->x = x;
    room->y = y;
    room->w = w;
    room->h = h;

    TCOD_list_push(map->rooms, room);

    return room;
}

void room_get_random_pos(room_t *room, int *x, int *y)
{
    *x = TCOD_random_get_int(NULL, room->x, room->x + room->w - 1);
    *y = TCOD_random_get_int(NULL, room->y, room->y + room->h - 1);
}

bool room_is_inside(room_t *room, int x, int y)
{
    return min(room->x, room->x + room->w) <= x && x < max(room->x, room->x + room->w) && min(room->y, room->y + room->h) <= y && y < max(room->y, room->y + room->h);
}

void room_destroy(room_t *room)
{
    free(room);
}