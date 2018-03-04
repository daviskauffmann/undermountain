#ifndef ROOM_H
#define ROOM_H

typedef struct room_s
{
    int x;
    int y;
    int w;
    int h;
} room_t;

room_t *room_create(int x, int y, int w, int h);
void room_get_random_pos(room_t *room, int *x, int *y);
void room_destroy(room_t *room);

#endif
