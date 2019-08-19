#include "message.h"

#include <malloc.h>
#include <stdio.h>
#include <string.h>

struct message *message_create(char *text, TCOD_color_t color)
{
    struct message *message = malloc(sizeof(struct message));

    if (!message)
    {
        printf("Couldn't allocate message\n");

        return NULL;
    }

    message->text = _strdup(text);
    message->color = color;

    return message;
}

void message_destroy(struct message *message)
{
    free(message->text);
    free(message);
}
