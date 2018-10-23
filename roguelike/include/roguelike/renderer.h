#ifndef RENDERER_H
#define RENDERER_H

#include <libtcod/libtcod.h>

struct game;
struct ui;

struct renderer
{
    TCOD_noise_t noise;
    float noise_x;
    TCOD_console_t message_log;
    TCOD_console_t panel;
    TCOD_console_t tooltip;
};

extern struct renderer *renderer;

void renderer_init(void);
void renderer_draw(void);
void renderer_quit(void);

#endif
