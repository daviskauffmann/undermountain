#ifndef CONFIG_H
#define CONFIG_H

int screen_width;
int screen_height;
bool fullscreen;
int renderer;

const char *font_file;
int font_flags;
int font_char_horiz;
int font_char_vertic;

void config_initialize(void);

#endif