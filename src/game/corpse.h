#ifndef GAME_CORPSE_H
#define GAME_CORPSE_H

#include <libtcod.h>

struct corpse_common
{
    unsigned char corpse_glyph;
    TCOD_color_t corpse_color;
};

struct corpse
{
    char *name;
    int level;
    int floor;
    int x;
    int y;
};

struct corpse *corpse_new(const char *name, int level, int floor, int x, int y);
void corpse_delete(struct corpse *corpse);

#endif
