#include <stdio.h>
#include <libtcod.h>

#include "input.h"
#include "config.h"
#include "console.h"
#include "world.h"
#include "game.h"

input_t input_handle(void)
{
    TCOD_key_t key;
    TCOD_mouse_t mouse;
    TCOD_event_t ev = TCOD_sys_check_for_event(TCOD_EVENT_ANY, &key, &mouse);

    switch (ev)
    {
    case TCOD_EVENT_MOUSE_PRESS:
        actor_target_set(player, mouse.cx + view_left, mouse.cy + view_top);

        return INPUT_TICK;

    case TCOD_EVENT_KEY_PRESS:
        actor_target_set(player, -1, -1);

        // TODO: find a place for this
        tile_t *tile_n = player->y - 1 > 0
                             ? &current_map->tiles[player->x][player->y - 1]
                             : NULL;
        bool walkable_n = tile_n == NULL ? false : tileinfo[tile_n->type].is_walkable;
        tile_t *tile_e = player->x + 1 < MAP_WIDTH
                             ? &current_map->tiles[player->x + 1][player->y]
                             : NULL;
        bool walkable_e = tile_e == NULL ? false : tileinfo[tile_e->type].is_walkable;
        tile_t *tile_s = player->y + 1 < MAP_HEIGHT
                             ? &current_map->tiles[player->x][player->y + 1]
                             : NULL;
        bool walkable_s = tile_s == NULL ? false : tileinfo[tile_s->type].is_walkable;
        tile_t *tile_w = player->x - 1 > 0
                             ? &current_map->tiles[player->x - 1][player->y]
                             : NULL;
        bool walkable_w = tile_w == NULL ? false : tileinfo[tile_w->type].is_walkable;

        switch (key.vk)
        {
        case TCODK_ESCAPE:
            return INPUT_QUIT;

        case TCODK_F1:
            right_panel_visible = !right_panel_visible;

            return INPUT_DRAW;

        case TCODK_F2:
            bottom_panel_visible = !bottom_panel_visible;

            return INPUT_DRAW;

        case TCODK_CHAR:
            switch (key.c)
            {
            case ',':
                if (key.shift)
                {
                    tile_t *tile = &current_map->tiles[player->x][player->y];
                    if (tile->type != TILETYPE_STAIR_UP)
                    {
                        // return INPUT_TICK;
                    }

                    if (current_map_index <= 0)
                    {
                        return INPUT_QUIT;
                    }

                    current_map_index--;

                    map_t *new_map = TCOD_list_get(maps, current_map_index);

                    TCOD_list_remove(current_map->actors, player);
                    TCOD_list_push(new_map->actors, player);

                    player->x = new_map->stair_down_x;
                    player->y = new_map->stair_down_y;

                    current_map = new_map;

                    return INPUT_TURN;
                }

                return INPUT_TICK;

            case '.':
                if (key.shift)
                {
                    tile_t *tile = &current_map->tiles[player->x][player->y];
                    if (tile->type != TILETYPE_STAIR_DOWN)
                    {
                        // return INPUT_TICK;
                    }

                    current_map_index++;

                    map_t *new_map = TCOD_list_size(maps) == current_map_index
                                         ? map_create()
                                         : TCOD_list_get(maps, current_map_index);

                    TCOD_list_remove(current_map->actors, player);
                    TCOD_list_push(new_map->actors, player);

                    player->x = new_map->stair_up_x;
                    player->y = new_map->stair_up_y;

                    current_map = new_map;

                    return INPUT_TURN;
                }

                return INPUT_TICK;

            case 'a':
                sfx = !sfx;

                return INPUT_DRAW;

            case 'b':
                return INPUT_TICK;

            case 'c':
                return INPUT_TICK;

            case 'd':
                return INPUT_TICK;

            case 'e':
                return INPUT_TICK;

            case 'f':
                return INPUT_TICK;

            case 'g':
                return INPUT_TICK;

            case 'h':
                return INPUT_TICK;

            case 'i':
                return INPUT_TICK;

            case 'j':
                return INPUT_TICK;

            case 'k':
                return INPUT_TICK;

            case 'l':
                if (key.lctrl)
                {
                    world_destroy();

                    console_init();
                    game_load();

                    return INPUT_DRAW;
                }

                return INPUT_TICK;

            case 'm':
                return INPUT_TICK;

            case 'n':
                return INPUT_TICK;

            case 'o':
                return INPUT_TICK;

            case 'p':
                return INPUT_TICK;

            case 'q':
                return INPUT_TICK;

            case 'r':
                world_destroy();

                console_init();
                world_init();
                game_init();

                return INPUT_DRAW;

            case 's':
                if (key.lctrl)
                {
                    game_save();
                }

                return INPUT_TICK;

            case 't':
                torch = !torch;

                if (torch)
                {
                    actorinfo[player->type].sight_radius *= 2;
                }
                else
                {
                    actorinfo[player->type].sight_radius /= 2;
                }

                return INPUT_DRAW;

            case 'u':
                return INPUT_TICK;

            case 'v':
                return INPUT_TICK;

            case 'w':
                return INPUT_TICK;

            case 'x':
                return INPUT_TICK;

            case 'y':
                torch_color = TCOD_color_RGB(TCOD_random_get_int(NULL, 0, 255), TCOD_random_get_int(NULL, 0, 255), TCOD_random_get_int(NULL, 0, 255));

                return INPUT_DRAW;

            case 'z':
                return INPUT_TICK;
            }

            return INPUT_TICK;

        case TCODK_KP1:
            if (walkable_s || walkable_w)
            {
                actor_move(current_map, player, player->x - 1, player->y + 1);
            }

            return INPUT_TURN;

        case TCODK_KP2:
        case TCODK_DOWN:
            actor_move(current_map, player, player->x, player->y + 1);

            return INPUT_TURN;

        case TCODK_KP3:
            if (walkable_e || walkable_s)
            {
                actor_move(current_map, player, player->x + 1, player->y + 1);
            }

            return INPUT_TURN;

        case TCODK_KP4:
        case TCODK_LEFT:
            actor_move(current_map, player, player->x - 1, player->y);

            return INPUT_TURN;

        case TCODK_KP5:
            return INPUT_TURN;

        case TCODK_KP6:
        case TCODK_RIGHT:
            actor_move(current_map, player, player->x + 1, player->y);

            return INPUT_TURN;

        case TCODK_KP7:
            if (walkable_n || walkable_w)
            {
                actor_move(current_map, player, player->x - 1, player->y - 1);
            }

            return INPUT_TURN;

        case TCODK_KP8:
        case TCODK_UP:
            actor_move(current_map, player, player->x, player->y - 1);

            return INPUT_TURN;

        case TCODK_KP9:
            if (walkable_n || walkable_e)
            {
                actor_move(current_map, player, player->x + 1, player->y - 1);
            }

            return INPUT_TURN;
        }

        return INPUT_TICK;
    }

    static float automove_timer = 0.0f;
    static bool automove_ready = true;

    if (automove_ready)
    {
        if (actor_target_moveto(current_map, player))
        {
            automove_ready = false;

            return INPUT_TURN;
        }
    }
    else
    {
        automove_timer += TCOD_sys_get_last_frame_length();

        if (automove_timer >= AUTOMOVE_DELAY)
        {
            automove_ready = true;

            automove_timer = 0.0f;
        }
    }

    return INPUT_TICK;
}