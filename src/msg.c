#include <libtcod.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>

#include "config.h"
#include "game.h"

void msg_init(void)
{
    messages = TCOD_list_new();
}

void msg_log(position_t *position, TCOD_color_t color, char *text, ...)
{
    position_t *player_position = (position_t *)component_get(player, COMPONENT_POSITION);
    fov_t *player_fov = (fov_t *)component_get(player, COMPONENT_FOV);

    if (position == NULL ||
        (position->map == player_position->map &&
         ((position->x == player_position->x && position->y == player_position->y) ||
          TCOD_map_is_in_fov(player_fov->fov_map, position->x, position->y))))
    {
        va_list ap;
        char buf[128];
        va_start(ap, text);
        vsprintf(buf, text, ap);
        va_end(ap);

        char *line_begin = buf;
        char *line_end;

        do
        {
            if (TCOD_list_size(messages) == (console_height / 4) - 2)
            {
                message_t *message = TCOD_list_get(messages, 0);

                TCOD_list_remove(messages, message);

                free(message->text);
                free(message);
            }

            line_end = strchr(line_begin, '\n');

            if (line_end)
            {
                *line_end = '\0';
            }

            message_t *message = (message_t *)malloc(sizeof(message_t));
            message->text = strdup(line_begin);
            message->color = color;

            TCOD_list_push(messages, message);

            line_begin = line_end + 1;
        } while (line_end);
    }
}

void msg_reset(void)
{
    for (void **iterator = TCOD_list_begin(messages); iterator != TCOD_list_end(messages); iterator++)
    {
        message_t *message = *iterator;

        free(message->text);
        free(message);
    }

    TCOD_list_delete(messages);
}