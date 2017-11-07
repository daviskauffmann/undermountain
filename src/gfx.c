#include <libtcod.h>

#include "CMemLeak.h"
#include "system.h"
#include "game.h"

#define CONSTRAIN_VIEW 1

void gfx_init()
{
    tile_glyph[TILE_TYPE_EMPTY] = ' ';
    tile_glyph[TILE_TYPE_FLOOR] = '.';
    tile_glyph[TILE_TYPE_WALL] = '#';
    tile_glyph[TILE_TYPE_STAIR_DOWN] = '>';
    tile_glyph[TILE_TYPE_STAIR_UP] = '<';

    background_color = TCOD_black;
    foreground_color = TCOD_white;
    tile_color_light = TCOD_white;
    tile_color_dark = TCOD_color_RGB(16, 16, 16);
    torch_color = TCOD_light_amber;

    msg_init();
    panel_init();
    tooltip_init();
}

void gfx_draw_turn(void)
{
    TCOD_console_set_default_background(NULL, background_color);
    TCOD_console_set_default_foreground(NULL, foreground_color);
    TCOD_console_clear(NULL);

    panel_width = screen_width / 2;
    panel_x = screen_width - panel_width;
    panel_y = 0;
    panel_height = screen_height;

    content_height[CONTENT_CHARACTER] = 18;
    content_height[CONTENT_INVENTORY] = TCOD_list_size(player->items) + 2;

    msg_x = 0;
    msg_height = screen_height / 4;
    msg_y = screen_height - msg_height;
    msg_width = screen_width - (panel_visible ? panel_width : 0);

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

    // TODO: calculate width an height based on content
    // we want each option to only take one line
    tooltip_width = 15;
    tooltip_height = 6;
    // TODO: keep this in the view
    tooltip_x = tooltip_tile_x - view_x;
    tooltip_y = tooltip_tile_y - view_y;

    world_draw_turn();
    msg_draw_turn();
    panel_draw_turn();
    tooltip_draw_turn();
}

void gfx_draw_tick(void)
{
    world_draw_tick();
    msg_draw_tick();
    panel_draw_tick();
    tooltip_draw_tick();

    TCOD_console_flush();
}

void gfx_uninit(void)
{
    msg_uninit();
    panel_uninit();
    tooltip_uninit();
}