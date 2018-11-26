#ifdef PLATFORM_LIBTCOD

#ifndef PLATFORM_LIBTCOD_CONFIG_H
#define PLATFORM_LIBTCOD_CONFIG_H

#include <libtcod/libtcod.h>

extern const char *font_file;
extern TCOD_font_flags_t font_flags;
extern int font_char_horiz;
extern int font_char_vertic;

extern bool fullscreen;
extern TCOD_renderer_t console_renderer;

void config_load(void);
void config_save(void);

#endif

#endif
