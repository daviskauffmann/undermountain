#include <libtcod.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>

#include "config.h"
#include "game.h"

void msg_log(game_t *game, position_t *position, TCOD_color_t color, char *text, ...)
{
    if (game->player != NULL)
    {
        position_t *player_position = (position_t *)component_get(game->player, COMPONENT_POSITION);
        fov_t *player_fov = (fov_t *)component_get(game->player, COMPONENT_FOV);

        if (player_position != NULL && player_fov != NULL)
        {
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
                    if (TCOD_list_size(game->state.messages) == (console_height / 4) - 2)
                    {
                        message_t *message = TCOD_list_get(game->state.messages, 0);

                        TCOD_list_remove(game->state.messages, message);

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

                    TCOD_list_push(game->state.messages, message);

                    line_begin = line_end + 1;
                } while (line_end);
            }
        }
    }
}