#ifndef RENDERER_H
#define RENDERER_H

#include <libtcod/libtcod.h>

struct game;
struct input;
struct ui;

struct renderer
{
    TCOD_noise_t noise;
    float noise_x;
    TCOD_console_t message_log;
    TCOD_console_t panel;
    TCOD_console_t tooltip;
};

struct renderer *renderer_create(void);
void renderer_draw(struct renderer *renderer, struct game *game, struct input *input, struct ui *ui);
void renderer_destroy(struct renderer *renderer);

#endif
