#ifndef CONSOLE_H
#define CONSOLE_H

#include <libtcod.h>

#include "world.h"

#define CONSTRAIN_VIEW 1

TCOD_console_t bottom_panel;
bool bottom_panel_visible;
int bottom_panel_x;
int bottom_panel_y;
int bottom_panel_width;
int bottom_panel_height;

TCOD_console_t right_panel;
bool right_panel_visible;
int right_panel_x;
int right_panel_y;
int right_panel_width;
int right_panel_height;

int view_left;
int view_top;
int view_right;
int view_bottom;

TCOD_list_t messages;

void console_init(void);
void console_log(const char *message, map_t *map, int x, int y);
void console_turn_draw(void);
void console_tick_draw(void);

#endif