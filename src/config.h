#ifndef WINDOW_H
#define WINDOW_H

#define WINDOW_TITLE "Roguelike v0.1"
#define FPS 60

#define SAVE_PATH "../saves/save.gz"

extern const char *font_file;
extern int font_flags;
extern int font_char_horiz;
extern int font_char_vertic;

extern int console_width;
extern int console_height;
extern bool fullscreen;
extern int renderer;

void config_init(void);

#endif
