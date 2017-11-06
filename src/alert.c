#include <libtcod.h>

#include "game.h"

void alert_show(char *message)
{
    alert_visible = true;

    alert_text = message;
}

void alert_draw_turn(void)
{
    if (!alert_visible)
    {
        return;
    }

    TCOD_console_set_default_background(alert, background_color);
    TCOD_console_set_default_foreground(alert, foreground_color);
    TCOD_console_clear(alert);

    TCOD_console_print_rect(alert, 1, 1, alert_width - 2, alert_height - 2, alert_text);

    TCOD_console_set_default_foreground(alert, foreground_color);
    TCOD_console_print_frame(alert, 0, 0, alert_width, alert_height, false, TCOD_BKGND_SET, "");
}

void alert_draw_tick(void)
{
    if (!alert_visible)
    {
        return;
    }

    TCOD_console_blit(alert, 0, 0, alert_width, alert_height, NULL, alert_x, alert_y, 1, 1);
}