#include <string.h>
#include <math.h>
#include <libtcod.h>

#include "CMemLeak.h"
#include "system.h"
#include "game.h"

void msg_log(const char *message, map_t *map, int x, int y)
{
    if (map != player->map)
    {
        return;
    }

    if (!TCOD_map_is_in_fov(player->fov_map, x, y))
    {
        return;
    }

    TCOD_list_push(messages, message);

    if (TCOD_list_size(messages) >= MAX_MESSAGES)
    {
        TCOD_list_remove(messages, *TCOD_list_begin(messages));
    }
}

void msg_draw(void)
{
    TCOD_list_t new_messages = TCOD_list_duplicate(messages);

    int total_lines = 0;
    for (void **i = TCOD_list_begin(messages); i != TCOD_list_end(messages); i++)
    {
        const char *message = *i;

        total_lines += (int)ceil((float)strlen(message) / (float)(msg_width - 2));
    }

    while (total_lines > msg_height - 2)
    {
        const char *message = *TCOD_list_begin(new_messages);

        TCOD_list_remove(new_messages, message);

        total_lines -= (int)ceil((float)strlen(message) / (float)(msg_width - 2));
    }

    int size = TCOD_list_size(new_messages);
    int index = 0;
    int message_y = 1;
    for (void **i = TCOD_list_begin(new_messages); i != TCOD_list_end(new_messages); i++)
    {
        index++;

        const char *message = *i;

        TCOD_color_t color = index == size
                                 ? TCOD_white
                                 : index == size - 1
                                       ? TCOD_light_gray
                                       : index == size - 2
                                             ? TCOD_gray
                                             : TCOD_dark_gray;

        TCOD_console_set_default_foreground(msg, color);
        message_y += TCOD_console_print_rect(msg, msg_x + 1, message_y, msg_width - 2, msg_height - 1, message);
    }

    TCOD_list_delete(new_messages);

    TCOD_console_set_default_foreground(msg, foreground_color);
    TCOD_console_print_frame(msg, 0, 0, msg_width, msg_height, false, TCOD_BKGND_SET, "Log");

    TCOD_console_blit(msg, 0, 0, msg_width, msg_height, NULL, msg_x, msg_y, 1, 1);
}