#ifndef CONFIG_H
#define CONFIG_H

#include <libtcod/libtcod.h>

#define WINDOW_TITLE "Roguelike v0.1"
#define FPS 60

extern const char *font_file;
extern TCOD_font_flags_t font_flags;
extern int font_char_horiz;
extern int font_char_vertic;

extern int console_width;
extern int console_height;
extern bool fullscreen;
extern TCOD_renderer_t console_renderer;

void config_load(void);
void config_save(void);

#endif
