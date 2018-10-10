#ifndef CONFIG_H
#define CONFIG_H

#include <libtcod/libtcod.h>

extern const char *font_file;
extern int font_flags;
extern int font_char_horiz;
extern int font_char_vertic;

extern int console_width;
extern int console_height;
extern bool fullscreen;
extern int console_renderer;

void config_load(void);
void config_save(void);

#endif
