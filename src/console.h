#ifndef CONSOLE_H
#define CONSOLE_H

#include <libtcod.h>

#include "map.h"

#define WINDOW_TITLE "Roguelike v0.1"
#define FPS 60

#define MAX_MESSAGES 20
#define CONSTRAIN_VIEW 1

typedef enum content_e {
    CONTENT_CHARACTER,
    CONTENT_INVENTORY,

    NUM_CONTENTS
} content_t;

TCOD_color_t background_color;
TCOD_color_t foreground_color;
TCOD_color_t tile_color_light;
TCOD_color_t tile_color_dark;
TCOD_color_t torch_color;

bool sfx;

int view_x;
int view_y;
int view_width;
int view_height;

TCOD_console_t message_log;
bool message_log_visible;
int message_log_x;
int message_log_y;
int message_log_width;
int message_log_height;
TCOD_list_t messages;

TCOD_console_t menu;
bool menu_visible;
int menu_x;
int menu_y;
int menu_width;
int menu_height;
content_t content;
int content_scroll[NUM_CONTENTS];
int content_height[NUM_CONTENTS];

void console_initialize(void);
void console_log(const char *message, map_t *map, int x, int y);
void console_turn_draw(void);
void console_tick_draw(void);
void console_finalize(void);

#endif