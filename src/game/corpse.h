#ifndef UM_GAME_CORPSE_H
#define UM_GAME_CORPSE_H

#include <libtcod.h>

struct corpse_metadata
{
    unsigned char glyph;
    TCOD_ColorRGB color;
};

struct corpse
{
    char *name;
    int level;

    int floor;
    int x;
    int y;
};

struct corpse *corpse_new(
    const char *name,
    int level,
    int floor,
    int x,
    int y);
void corpse_delete(struct corpse *corpse);

#endif
