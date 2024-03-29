#include "message.h"

#include <malloc.h>
#include <string.h>

struct message *message_new(
    const char *const text,
    const TCOD_ColorRGB color)
{
    struct message *const message = malloc(sizeof(*message));

    message->text = strdup(text);

    message->color = color;

    return message;
}

void message_delete(struct message *const message)
{
    free(message->text);

    free(message);
}
