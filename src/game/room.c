#include "room.h"

#include "world.h"
#include <assert.h>
#include <libtcod.h>
#include <malloc.h>

struct room *room_new(
    const uint8_t x,
    const uint8_t y,
    const uint8_t w,
    const uint8_t h)
{
    struct room *const room = malloc(sizeof(*room));
    assert(room);

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
        world->random,
        room->x,
        room->x + room->w - 1);
    *y = TCOD_random_get_int(
        world->random,
        room->y,
        room->y + room->h - 1);
}
