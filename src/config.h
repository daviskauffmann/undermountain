#ifndef CONFIG_H
#define CONFIG_H

#include <libtcod.h>

#include "world.h"

#define SCREEN_WIDTH 80
#define SCREEN_HEIGHT 50
#define WINDOW_TITLE "Roguelike v0.1"

typedef struct
{
    unsigned char glyph;
    TCOD_color_t color;
    bool is_transparent;
    bool is_walkable;
} tileinfo_t;

typedef struct
{
    unsigned char glyph;
    TCOD_color_t color;
    int sight_radius;
} actorinfo_t;

extern tileinfo_t tileinfo[NUM_TILETYPES];
extern actorinfo_t actorinfo[NUM_ACTORTYPES];

void config_init(void);

#endif