#ifndef MESSAGE_H
#define MESSAGE_H

#include <libtcod.h>

struct message
{
    char *text;
    TCOD_color_t color; // TODO: change to message type, let an asset or something decide the color
};

struct message *message_new(char *text, TCOD_color_t color);
void message_delete(struct message *message);

#endif
