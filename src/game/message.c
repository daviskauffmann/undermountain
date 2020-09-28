#include "message.h"

#include <assert.h>
#include <malloc.h>

struct message *message_new(char *text, TCOD_color_t color)
{
    struct message *message = malloc(sizeof *message);
    assert(message);
    message->text = TCOD_strdup(text);
    message->color = color;
    return message;
}

void message_delete(struct message *message)
{
    free(message->text);
    free(message);
}
