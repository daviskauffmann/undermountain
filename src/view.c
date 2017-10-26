#include <libtcod.h>

#include "view.h"
#include "config.h"
#include "game.h"
#include "world.h"

void view_update(void)
{
    view_right = SCREEN_WIDTH;
    view_bottom = SCREEN_HEIGHT;
    view_left = player->x - view_right / 2;
    view_top = player->y - view_bottom / 2;

    if (CONSTRAIN_VIEW)
    {
        view_left = view_left < 0
                        ? 0
                        : view_left + view_right > MAP_WIDTH
                              ? MAP_WIDTH - view_right
                              : view_left;
        view_top = view_top < 0
                       ? 0
                       : view_top + view_bottom > MAP_HEIGHT
                             ? MAP_HEIGHT - view_bottom
                             : view_top;
    }
}

void view_render(void)
{
    TCOD_map_t TCOD_map = map_to_TCOD_map(current_map);
    TCOD_map = map_calc_fov(TCOD_map, player->x, player->y, actorinfo[player->type].sight_radius);

    TCOD_console_clear(NULL);

    for (int x = view_left; x < view_left + view_right; x++)
    {
        for (int y = view_top; y < view_top + view_bottom; y++)
        {
            if (x < 0 || x >= MAP_WIDTH || y < 0 || y >= MAP_HEIGHT)
            {
                continue;
            }

            tile_t *tile = &current_map->tiles[x][y];

            TCOD_color_t color;
            if (TCOD_map_is_in_fov(TCOD_map, x, y))
            {
                tile->seen = true;

                color = tileinfo[tile->type].color;
            }
            else
            {
                if (tile->seen)
                {
                    color = TCOD_gray;
                }
                else
                {
                    continue;
                }
            }

            TCOD_console_set_char_foreground(NULL, x - view_left, y - view_top, color);
            TCOD_console_set_char(NULL, x - view_left, y - view_top, tileinfo[tile->type].glyph);
        }
    }

    for (actor_t **iterator = (actor_t **)TCOD_list_begin(current_map->actors);
         iterator != (actor_t **)TCOD_list_end(current_map->actors);
         iterator++)
    {
        actor_t *actor = *iterator;

        if (!TCOD_map_is_in_fov(TCOD_map, actor->x, actor->y))
        {
            continue;
        }

        TCOD_console_set_char_foreground(NULL, actor->x - view_left, actor->y - view_top, actorinfo[actor->type].color);
        TCOD_console_set_char(NULL, actor->x - view_left, actor->y - view_top, actorinfo[actor->type].glyph);
    }

    TCOD_console_flush();

    TCOD_map_delete(TCOD_map);
}