#ifndef CONSOLE_H
#define CONSOLE_H

#include <libtcod.h>

#include "world.h"

#define MAX_MESSAGES 20
#define CONSTRAIN_VIEW 1

typedef enum content_type_e {
    CONTENT_NONE = 0,
    CONTENT_CHARACTER,
    CONTENT_INVENTORY,

    NUM_CONTENT_TYPES
} content_type_t;

typedef struct content_s
{
    content_type_t type;
    int height;
    int scroll;
} content_t;

TCOD_console_t right_panel;
bool right_panel_visible;
int right_panel_x;
int right_panel_y;
int right_panel_width;
int right_panel_height;
content_t right_panel_content[NUM_CONTENT_TYPES];
content_type_t right_panel_content_type;

TCOD_console_t message_log;
bool message_log_visible;
int message_log_x;
int message_log_y;
int message_log_width;
int message_log_height;
TCOD_list_t messages;

int view_left;
int view_top;
int view_right;
int view_bottom;

void console_initialize(void);
void console_log(const char *message, map_t *map, int x, int y);
void console_turn_draw(void);
void console_tick_draw(void);
void console_finalize(void);

#endif