#include <libtcod.h>

#include "game.h"

void tooltip_draw_turn(void)
{
    if (!tooltip_visible)
    {
        return;
    }

    TCOD_console_set_default_background(tooltip, background_color);
    TCOD_console_set_default_foreground(tooltip, foreground_color);
    TCOD_console_clear(tooltip);

    TCOD_console_print_rect(tooltip, 1, 1, tooltip_width - 2, tooltip_height - 2, "Tile: ");
    TCOD_console_print_rect(tooltip, 1, 2, tooltip_width - 2, tooltip_height - 2, "Light: ");
    TCOD_console_print_rect(tooltip, 1, 3, tooltip_width - 2, tooltip_height - 2, "Actor: ");
    TCOD_console_print_rect(tooltip, 1, 4, tooltip_width - 2, tooltip_height - 2, "Items: ");

    TCOD_console_set_default_foreground(tooltip, foreground_color);
    TCOD_console_print_frame(tooltip, 0, 0, tooltip_width, tooltip_height, false, TCOD_BKGND_SET, "");
}

void tooltip_draw_tick(void)
{
    if (!tooltip_visible)
    {
        return;
    }

    TCOD_console_blit(tooltip, 0, 0, tooltip_width, tooltip_height, NULL, tooltip_x - view_x, tooltip_y - view_y, 1, 1);
}