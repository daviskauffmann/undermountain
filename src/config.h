#ifndef CONFIG_H
#define CONFIG_H

#include <libtcod.h>

#include "world.h"

#define WINDOW_TITLE "Roguelike v0.1"
#define FPS 60

int screen_width;
int screen_height;
bool fullscreen;
int renderer;

const char *font_file;
int font_flags;
int font_char_horiz;
int font_char_vertic;

bool sfx;
TCOD_color_t default_background_color;
TCOD_color_t default_foreground_color;
TCOD_color_t torch_color;

tile_info_t tile_info[NUM_TILE_TYPES];
actor_info_t actor_info[NUM_ACTOR_TYPES];

void config_initialize(void);

#endif