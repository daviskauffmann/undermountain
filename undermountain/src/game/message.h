#ifndef GAME_MESSAGE_H
#define GAME_MESSAGE_H

#include <libtcod.h>

struct message
{
    char *text;
    TCOD_color_t color; // TODO: change to message type, let an asset or something decide the color
};

struct message *message_create(char *text, TCOD_color_t color);
void message_destroy(struct message *message);

#endif
