#ifndef WINDOW_H
#define WINDOW_H

#include <libtcod/libtcod.h>

#define WINDOW_TITLE "Roguelike v0.1"
#define FPS 60

extern const char *font_file;
extern int font_flags;
extern int font_char_horiz;
extern int font_char_vertic;

extern int console_width;
extern int console_height;
extern bool fullscreen;
extern int console_renderer;

void window_init(void);
bool window_is_closed(void);
void window_quit(void);

#endif
