#ifndef GAME_MESSAGE_H
#define GAME_MESSAGE_H

#include <libtcod.h>

struct message
{
    char *text;
    TCOD_ColorRGB color; // TODO: change to message type, let an asset or something decide the color
};

struct message *message_new(
    const char *text,
    TCOD_ColorRGB color);
void message_delete(struct message *message);

#endif
