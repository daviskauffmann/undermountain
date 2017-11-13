#include <string.h>
#include <math.h>
#include <libtcod.h>
#include <stdarg.h>
#include <stdio.h>

#include "CMemLeak.h"
#include "system.h"
#include "game.h"

static TCOD_console_t msg;
static TCOD_list_t messages;

message_t *message_create(char *text, TCOD_color_t color)
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

void msg_init(void)
{
    msg = TCOD_console_new(screen_width, screen_height);
    msg_visible = true;
    messages = TCOD_list_new();
}

void msg_log(map_t *map, int x, int y, TCOD_color_t color, char *text, ...)
{
    if (map == player->map && TCOD_map_is_in_fov(player->fov_map, x, y))
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
            if (TCOD_list_size(messages) == 4)
            {
                message_t *message = TCOD_list_get(messages, 0);

                TCOD_list_remove(messages, message);

                message_destroy(message);
            }

            line_end = strchr(line_begin, '\n');

            if (line_end)
            {
                *line_end = '\0';
            }

            message_t *message = message_create(line_begin, color);

            TCOD_list_push(messages, message);

            line_begin = line_end + 1;
        } while (line_end);
    }
}

bool msg_is_inside(int x, int y)
{
    return x >= msg_x && x < msg_x + msg_width && y >= msg_y && y < msg_y + msg_height;
}

void msg_draw_turn(void)
{
    if (msg_visible)
    {
        TCOD_console_set_default_background(msg, background_color);
        TCOD_console_set_default_foreground(msg, foreground_color);
        TCOD_console_clear(msg);

        int y = 1;
        for (void **i = TCOD_list_begin(messages); i != TCOD_list_end(messages); i++)
        {
            message_t *message = *i;

            TCOD_console_set_default_foreground(msg, message->color);
            TCOD_console_print(msg, msg_x + 1, y, message->text);

            y++;
        }

        TCOD_console_set_default_foreground(msg, foreground_color);
        TCOD_console_print_frame(msg, 0, 0, msg_width, msg_height, false, TCOD_BKGND_SET, "Log");
    }
}

void msg_draw_tick(void)
{
    if (msg_visible)
    {
        TCOD_console_blit(msg, 0, 0, msg_width, msg_height, NULL, msg_x, msg_y, 1, 1);
    }
}

void msg_uninit(void)
{
    for (void **i = TCOD_list_begin(messages); i != TCOD_list_end(messages); i++)
    {
        message_t *message = *i;

        message_destroy(message);
    }

    TCOD_list_delete(messages);

    TCOD_console_delete(msg);
}