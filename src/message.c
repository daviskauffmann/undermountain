#include <libtcod/libtcod.h>
#include <malloc.h>
#include <string.h>

#include "message.h"

struct message *message_create(const char *text, TCOD_color_t color)
{
    struct message *message = malloc(sizeof(struct message));

    message->text = strdup(text);
    message->color = color;

    return message;
}

void message_destroy(struct message *message)
{
    free(message->text);
    free(message);
}
