#ifndef MESSAGE_H
#define MESSAGE_H

#include <libtcod/libtcod.h>

typedef struct message_s message_t;

struct message_s
{
    char *text;
    TCOD_color_t color;
};

message_t *message_create(char *text, TCOD_color_t color);
void message_destroy(message_t *message);

#endif
