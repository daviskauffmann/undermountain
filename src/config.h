#ifndef CONFIG_H
#define CONFIG_H

#include <libtcod.h>

void config_init(void);

/* Window */
#define WINDOW_TITLE "Roguelike v0.1"
#define FPS 60

int screen_width;
int screen_height;
bool fullscreen;
int renderer;

/* Fonts */
char *font_file;
int font_flags;
int font_char_horiz;
int font_char_vertic;

#endif