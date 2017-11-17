#ifndef GAME_H
#define GAME_H

#include <libtcod.h>

#include "ECS.h"

typedef enum game_status_e {
    STATUS_WAITING,
    STATUS_UPDATE,
    STATUS_QUIT
} game_status_t;

game_status_t game_status;
int turn;
entity_t *player;

void game_init(void);
void game_new(void);
void game_reset(void);

/* Message Log */
#include <libtcod.h>

#include "ECS.h"

typedef struct message_s
{
    char *text;
    TCOD_color_t color;
} message_t;

TCOD_console_t msg;
TCOD_list_t messages;

message_t *message_create(char *text, TCOD_color_t color);
void message_destroy(message_t *message);
void msg_log(position_t *position, TCOD_color_t color, char *text, ...);

#endif