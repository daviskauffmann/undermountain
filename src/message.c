#include <libtcod/libtcod.h>
#include <string.h>

#include "message.h"

message_t *
message_create(char *text, TCOD_color_t color)
{
    message_t *message = (message_t *)malloc(sizeof(message_t));

    message->text = strdup(text);
    message->color = color;

    return message;
}

void message_destroy(message_t *message)
{
    free(message->text);

    free(message);
}
