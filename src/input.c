#include <libtcod.h>

#include "input.h"
#include "config.h"
#include "console.h"
#include "world.h"
#include "game.h"

static void automove_cancel(void);

static int automove_x = -1;
static int automove_y = -1;
static actor_t *automove_actor = NULL;

input_t input_handle(void)
{
    TCOD_key_t key;
    TCOD_mouse_t mouse;
    TCOD_event_t ev = TCOD_sys_check_for_event(TCOD_EVENT_ANY, &key, &mouse);

    switch (ev)
    {
    case TCOD_EVENT_MOUSE_PRESS:
        if (mouse.lbutton)
        {
            automove_cancel();

            int mouse_x = mouse.cx + view_left;
            int mouse_y = mouse.cy + view_top;

            tile_t *tile = &player->map->tiles[mouse_x][mouse_y];
            actor_t *actor = tile->actor;
            if (actor != NULL)
            {
                automove_actor = actor;
            }
            else
            {
                automove_x = mouse_x;
                automove_y = mouse_y;
            }
        }
        else if (mouse.rbutton)
        {
        }
        else if (mouse.wheel_down)
        {
            if (right_panel_content[right_panel_content_type].scroll + right_panel_height < right_panel_content[right_panel_content_type].height)
            {
                right_panel_content[right_panel_content_type].scroll++;
            }

            return INPUT_DRAW;
        }
        else if (mouse.wheel_up)
        {
            if (right_panel_content[right_panel_content_type].scroll > 0)
            {
                right_panel_content[right_panel_content_type].scroll--;
            }

            return INPUT_DRAW;
        }

        return INPUT_TICK;

    case TCOD_EVENT_KEY_PRESS:
        automove_cancel();

        // TODO: find a place for this
        tile_t *tile_n = player->y - 1 > 0
                             ? &player->map->tiles[player->x][player->y - 1]
                             : NULL;
        bool walkable_n = tile_n == NULL
                              ? false
                              : tile_info[tile_n->type].is_walkable;
        tile_t *tile_e = player->x + 1 < MAP_WIDTH
                             ? &player->map->tiles[player->x + 1][player->y]
                             : NULL;
        bool walkable_e = tile_e == NULL
                              ? false
                              : tile_info[tile_e->type].is_walkable;
        tile_t *tile_s = player->y + 1 < MAP_HEIGHT
                             ? &player->map->tiles[player->x][player->y + 1]
                             : NULL;
        bool walkable_s = tile_s == NULL
                              ? false
                              : tile_info[tile_s->type].is_walkable;
        tile_t *tile_w = player->x - 1 > 0
                             ? &player->map->tiles[player->x - 1][player->y]
                             : NULL;
        bool walkable_w = tile_w == NULL
                              ? false
                              : tile_info[tile_w->type].is_walkable;

        switch (key.vk)
        {
        case TCODK_ESCAPE:
            return INPUT_QUIT;

        case TCODK_PAGEDOWN:
            if (right_panel_content[right_panel_content_type].scroll + right_panel_height < right_panel_content[right_panel_content_type].height)
            {
                right_panel_content[right_panel_content_type].scroll++;
            }

            return INPUT_DRAW;

        case TCODK_PAGEUP:
            if (right_panel_content[right_panel_content_type].scroll > 0)
            {
                right_panel_content[right_panel_content_type].scroll--;
            }

            return INPUT_DRAW;

        case TCODK_CHAR:
            switch (key.c)
            {
            case ',':
                if (key.shift)
                {
                    tile_t *tile = &player->map->tiles[player->x][player->y];
                    if (tile->type != TILE_STAIR_UP)
                    {
                        // return INPUT_TICK;
                    }

                    if (current_map_index <= 0)
                    {
                        return INPUT_QUIT;
                    }

                    current_map_index--;

                    map_t *new_map = TCOD_list_get(maps, current_map_index);

                    TCOD_list_remove(player->map->actors, player);
                    TCOD_list_push(new_map->actors, player);

                    player->map->tiles[player->x][player->y].actor = NULL;
                    new_map->tiles[new_map->stair_down_x][new_map->stair_down_y].actor = player;

                    player->x = new_map->stair_down_x;
                    player->y = new_map->stair_down_y;

                    player->map = new_map;

                    return INPUT_TURN;
                }

                return INPUT_TICK;

            case '.':
                if (key.shift)
                {
                    tile_t *tile = &player->map->tiles[player->x][player->y];
                    if (tile->type != TILE_STAIR_DOWN)
                    {
                        // return INPUT_TICK;
                    }

                    current_map_index++;

                    map_t *new_map = TCOD_list_size(maps) == current_map_index
                                         ? map_create()
                                         : TCOD_list_get(maps, current_map_index);

                    TCOD_list_remove(player->map->actors, player);
                    TCOD_list_push(new_map->actors, player);

                    player->map->tiles[player->x][player->y].actor = NULL;
                    new_map->tiles[new_map->stair_up_x][new_map->stair_up_y].actor = player;

                    player->x = new_map->stair_up_x;
                    player->y = new_map->stair_up_y;

                    player->map = new_map;

                    return INPUT_TURN;
                }

                return INPUT_TICK;

            case 'a':
                sfx = !sfx;

                return INPUT_DRAW;

            case 'b':
                return INPUT_TICK;

            case 'c':
                if (right_panel_visible && right_panel_content_type == CONTENT_CHARACTER)
                {
                    right_panel_visible = false;
                }
                else
                {
                    right_panel_content_type = CONTENT_CHARACTER;

                    right_panel_visible = true;
                }

                return INPUT_DRAW;

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
                if (right_panel_visible && right_panel_content_type == CONTENT_INVENTORY)
                {
                    right_panel_visible = false;
                }
                else
                {
                    right_panel_content_type = CONTENT_INVENTORY;

                    right_panel_visible = true;
                }

                return INPUT_DRAW;

            case 'j':
                return INPUT_TICK;

            case 'k':
                return INPUT_TICK;

            case 'l':
                if (key.lctrl)
                {
                    console_finalize();
                    world_finalize();
                    game_finalize();

                    game_load();

                    return INPUT_DRAW;
                }

                message_log_visible = !message_log_visible;

                return INPUT_DRAW;

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
                console_finalize();
                world_finalize();
                game_finalize();

                console_initialize();
                world_initialize();
                game_initialize();

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
                    actor_info[player->type].sight_radius *= 2;
                }
                else
                {
                    actor_info[player->type].sight_radius /= 2;
                }

                actor_calc_fov(player);

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
                return actor_move(player, player->x - 1, player->y + 1)
                           ? INPUT_TURN
                           : INPUT_DRAW;
            }

            return INPUT_TICK;

        case TCODK_KP2:
        case TCODK_DOWN:
            return actor_move(player, player->x, player->y + 1)
                       ? INPUT_TURN
                       : INPUT_DRAW;

        case TCODK_KP3:
            if (walkable_e || walkable_s)
            {
                return actor_move(player, player->x + 1, player->y + 1)
                           ? INPUT_TURN
                           : INPUT_DRAW;
            }

            return INPUT_TICK;

        case TCODK_KP4:
        case TCODK_LEFT:
            return actor_move(player, player->x - 1, player->y)
                       ? INPUT_TURN
                       : INPUT_DRAW;

        case TCODK_KP5:
            return INPUT_TURN;

        case TCODK_KP6:
        case TCODK_RIGHT:
            return actor_move(player, player->x + 1, player->y)
                       ? INPUT_TURN
                       : INPUT_DRAW;

        case TCODK_KP7:
            if (walkable_n || walkable_w)
            {
                return actor_move(player, player->x - 1, player->y - 1)
                           ? INPUT_TURN
                           : INPUT_DRAW;
            }

            return INPUT_TICK;

        case TCODK_KP8:
        case TCODK_UP:
            return actor_move(player, player->x, player->y - 1)
                       ? INPUT_TURN
                       : INPUT_DRAW;

        case TCODK_KP9:
            if (walkable_n || walkable_e)
            {
                return actor_move(player, player->x + 1, player->y - 1)
                           ? INPUT_TURN
                           : INPUT_DRAW;
            }

            return INPUT_TICK;
        }

        return INPUT_TICK;
    }

    static float automove_timer = 0.0f;
    static bool automove_ready = true;

    if (automove_ready)
    {
        int x = automove_actor == NULL
                    ? automove_x
                    : automove_actor->x;
        int y = automove_actor == NULL
                    ? automove_y
                    : automove_actor->y;

        if (actor_move_towards(player, x, y))
        {
            automove_ready = false;

            return INPUT_TURN;
        }
        else
        {
            automove_cancel();
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

static void automove_cancel(void)
{
    automove_x = -1;
    automove_y = -1;
    automove_actor = NULL;
}