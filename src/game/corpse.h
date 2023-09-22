#ifndef UM_GAME_CORPSE_H
#define UM_GAME_CORPSE_H

#include <libtcod.h>

struct corpse_common
{
    unsigned char glyph;
    TCOD_ColorRGB color;
};

struct corpse
{
    char *name;
    uint8_t level;

    uint8_t floor;
    uint8_t x;
    uint8_t y;
};

struct corpse *corpse_new(
    const char *name,
    uint8_t level,
    uint8_t floor,
    uint8_t x,
    uint8_t y);
void corpse_delete(struct corpse *corpse);

#endif
