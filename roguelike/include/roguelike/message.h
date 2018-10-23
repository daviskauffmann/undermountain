#ifndef MESSAGE_H
#define MESSAGE_H

#include <libtcod/libtcod.h>

struct message
{
    char *text;
    TCOD_color_t color;
};

struct message *message_create(char *text, TCOD_color_t color);
void message_destroy(struct message *message);

#endif
