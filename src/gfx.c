#include <libtcod.h>

#include "CMemLeak.h"
#include "system.h"
#include "game.h"

#define CONSTRAIN_VIEW 1

void gfx_init(void)
{
    background_color = TCOD_black;
    foreground_color = TCOD_white;
    tile_color_light = TCOD_white;
    tile_color_dark = TCOD_color_RGB(16, 16, 32);

    msg_init();
    panel_init();
}

void gfx_draw(void)
{
    msg_x = 0;
    msg_height = screen_height / 4;
    msg_y = screen_height - msg_height;
    msg_width = screen_width;

    panel_width = screen_width / 2;
    panel_x = screen_width - panel_width;
    panel_y = 0;
    panel_height = screen_height - (msg_visible ? msg_height : 0);

    content_height[CONTENT_CHARACTER] = 18;
    content_height[CONTENT_INVENTORY] = TCOD_list_size(player->items) + 2;

    view_width = screen_width - (panel_visible ? panel_width : 0);
    view_height = screen_height - (msg_visible ? msg_height : 0);
    view_x = player->x - view_width / 2;
    view_y = player->y - view_height / 2;

#if CONSTRAIN_VIEW
    view_x = view_x < 0
                 ? 0
                 : view_x + view_width > MAP_WIDTH
                       ? MAP_WIDTH - view_width
                       : view_x;
    view_y = view_y < 0
                 ? 0
                 : view_y + view_height > MAP_HEIGHT
                       ? MAP_HEIGHT - view_height
                       : view_y;
#endif

    world_draw();
    msg_draw();
    panel_draw();

    TCOD_console_flush();
}

void gfx_uninit(void)
{
    msg_uninit();
    panel_uninit();
}