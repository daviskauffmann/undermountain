#ifndef ENGINE_H
#define ENGINE_H

#define WINDOW_TITLE "Roguelike v0.1"
#define FPS 60

extern int screen_width;
extern int screen_height;
extern int console_width;
extern int console_height;
extern int fullscreen;
extern int renderer;

extern const char *font_file;
extern int font_flags;
extern int font_char_horiz;
extern int font_char_vertic;

extern void
engine_init(void);

extern void
engine_quit(void);

#endif
