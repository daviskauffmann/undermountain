#include <libtcod.h>

#include "render.h"
#include "config.h"
#include "world.h"

void render(world_t *world)
{
    map_t *map = world->current_map;
    actor_t *player = world->player;

    TCOD_map_t TCOD_map = map_to_TCOD_map(map);

    map_calc_fov(TCOD_map, player->x, player->y, player->sight_radius);

    TCOD_console_clear(NULL);

    int vw = SCREEN_WIDTH;
    int vh = SCREEN_HEIGHT;
    int vx = player->x - vw / 2;
    int vy = player->y - vh / 2;

    if (CONSTRAIN_VIEW)
    {
        vx = vx < 0
                 ? 0
                 : vx + vw > MAP_WIDTH
                       ? MAP_WIDTH - vw
                       : vx;
        vy = vy < 0
                 ? 0
                 : vy + vh > MAP_HEIGHT
                       ? MAP_HEIGHT - vh
                       : vy;
    }

    for (int x = vx; x < vx + vw; x++)
    {
        for (int y = vy; y < vy + vh; y++)
        {
            if (x < 0 || x >= MAP_WIDTH || y < 0 || y >= MAP_HEIGHT)
            {
                continue;
            }

            tile_t *tile = &map->tiles[x][y];

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

            TCOD_console_set_char_foreground(NULL, x - vx, y - vy, color);
            TCOD_console_set_char(NULL, x - vx, y - vy, tileinfo[tile->type].glyph);
        }
    }

    for (actor_t **iterator = (actor_t **)TCOD_list_begin(map->actors);
         iterator != (actor_t **)TCOD_list_end(map->actors);
         iterator++)
    {
        actor_t *actor = *iterator;

        if (!TCOD_map_is_in_fov(TCOD_map, actor->x, actor->y))
        {
            continue;
        }

        TCOD_console_set_char_foreground(NULL, actor->x - vx, actor->y - vy, actor->color);
        TCOD_console_set_char(NULL, actor->x - vx, actor->y - vy, actor->glyph);
    }

    TCOD_console_flush();

    TCOD_map_delete(TCOD_map);
}