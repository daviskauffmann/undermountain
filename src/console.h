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
    int height;
    int scroll;
} content_t;

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
content_t menu_content[NUM_CONTENT_TYPES];
content_type_t menu_content_type;

void console_initialize(void);
void console_log(const char *message, map_t *map, int x, int y);
void console_turn_draw(void);
void console_tick_draw(void);
void console_finalize(void);

#endif