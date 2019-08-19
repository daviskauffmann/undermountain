#ifndef PLATFORM_CONFIG_H
#define PLATFORM_CONFIG_H

#include <libtcod.h>

#define TITLE "Undermountain"
#define VERSION "0.0.1"
#define FPS 60
#define SAVE_PATH "save.gz"

extern const char *font_file;
extern TCOD_font_flags_t font_flags;
extern int font_char_horiz;
extern int font_char_vertic;

extern bool fullscreen;
extern TCOD_renderer_t console_renderer;
extern int console_width;
extern int console_height;

void config_load(void);
void config_save(void);

#endif
