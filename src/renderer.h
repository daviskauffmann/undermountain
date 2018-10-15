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

struct renderer *renderer_create(void);
void renderer_draw(struct renderer *renderer, struct program *program, struct game *game, struct ui *ui);
void renderer_destroy(struct renderer *renderer);

#endif
