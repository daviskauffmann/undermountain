#ifndef SCENES_GAME_RECT_H
#define SCENES_GAME_RECT_H

#include <libtcod.h>

struct rect
{
    TCOD_Console *console;
    bool visible;
    int x;
    int y;
    int width;
    int height;
};

bool rect_is_inside(const struct rect rect, int x, int y);

#endif
