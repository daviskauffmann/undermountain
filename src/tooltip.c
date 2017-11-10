#include <stdio.h>
#include <libtcod.h>

#include "CMemLeak.h"
#include "system.h"
#include "game.h"

static TCOD_console_t tooltip;

void tooltip_init(void)
{
    tooltip = TCOD_console_new(screen_width, screen_height);
    tooltip_visible = false;
    tooltip_options = TCOD_list_new();
}

void tooltip_show(int x, int y)
{
    tooltip_options_clear();

    tooltip_visible = true;
    tooltip_x = x;
    tooltip_y = y;
}

void tooltip_hide()
{
    tooltip_options_clear();

    tooltip_visible = false;
}

bool tooltip_is_inside(int x, int y)
{
    return x >= tooltip_x && x < tooltip_x + tooltip_width && y >= tooltip_y && y < tooltip_y + tooltip_height;
}

void tooltip_draw_turn(void)
{
    if (!tooltip_visible)
    {
        return;
    }

    TCOD_console_set_default_background(tooltip, background_color);
    TCOD_console_set_default_foreground(tooltip, foreground_color);
    TCOD_console_clear(tooltip);

    TCOD_console_set_default_foreground(tooltip, foreground_color);
    TCOD_console_print_frame(tooltip, 0, 0, tooltip_width, tooltip_height, false, TCOD_BKGND_SET, "");
}

void tooltip_draw_tick(void)
{
    if (!tooltip_visible)
    {
        return;
    }

    int y = 1;
    for (void **i = TCOD_list_begin(tooltip_options); i != TCOD_list_end(tooltip_options); i++)
    {
        tooltip_option_t *option = *i;

        TCOD_color_t color = TCOD_white;

        if (mouse_x >= tooltip_x && mouse_x < tooltip_x + tooltip_width && mouse_y >= tooltip_y && mouse_y < tooltip_y + tooltip_height)
        {
            if (mouse_y == y + tooltip_y)
            {
                color = TCOD_yellow;
            }
        }

        TCOD_console_set_default_foreground(tooltip, color);
        y += TCOD_console_print_rect(tooltip, 1, y, tooltip_width - 2, tooltip_height - 2, option->text);
    }

    TCOD_console_blit(tooltip, 0, 0, tooltip_width, tooltip_height, NULL, tooltip_x, tooltip_y, 1, 1);
}

void tooltip_uninit(void)
{
    TCOD_console_delete(tooltip);

    for (void **i = TCOD_list_begin(tooltip_options); i != TCOD_list_end(tooltip_options); i++)
    {
        tooltip_option_t *option = *i;

        free(option);
    }

    TCOD_list_delete(tooltip_options);
}