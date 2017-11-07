#ifndef SYSTEM_H
#define SYSTEM_H

/* Window */
#define WINDOW_TITLE "Roguelike v0.1"
#define FPS 60

int screen_width;
int screen_height;
bool fullscreen;
int renderer;

/* Fonts */
const char *font_file;
int font_flags;
int font_char_horiz;
int font_char_vertic;

/* Graphics */
bool sfx;

void system_init(void);

#endif