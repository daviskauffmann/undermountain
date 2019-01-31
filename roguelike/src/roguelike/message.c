#include <roguelike.h>

struct message *message_create(char *text, TCOD_color_t color)
{
    struct message *message = malloc(sizeof(struct message));

    message->text = _strdup(text);
    message->color = color;

    return message;
}

void message_destroy(struct message *message)
{
    free(message->text);
    free(message);
}
