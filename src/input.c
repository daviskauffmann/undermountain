#include <libtcod.h>
#include <stdio.h>

#include "CMemLeak.h"
#include "system.h"
#include "game.h"

#define AUTOMOVE_DELAY 0.05f

static bool automove;
static int automove_x;
static int automove_y;
static actor_t *automove_actor;
static bool automove_attack;
static bool automove_take_items;

static void automove_set(int x, int y, bool attack, bool take_items);
static void automove_clear();

void input_init(void)
{
    automove = false;
    automove_x = -1;
    automove_y = -1;
    automove_actor = NULL;
    automove_attack = false;
    automove_take_items = false;
}

game_input_t input_handle(void)
{
    TCOD_key_t key;
    TCOD_mouse_t mouse;
    TCOD_event_t ev = TCOD_sys_check_for_event(TCOD_EVENT_ANY, &key, &mouse);

    mouse_x = mouse.cx;
    mouse_y = mouse.cy;
    mouse_tile_x = mouse.cx + view_x;
    mouse_tile_y = mouse.cy + view_y;

    switch (ev)
    {
    case TCOD_EVENT_MOUSE_PRESS:
        if (mouse.lbutton)
        {
            if (mouse_tile_x < view_x || mouse_tile_x >= view_x + view_width || mouse_tile_y < view_y || mouse_tile_y >= view_y + view_height)
            {
                return GAME_INPUT_TICK;
            }

            if (tooltip_visible)
            {
                if (mouse_x >= tooltip_x && mouse_x < tooltip_x + tooltip_width && mouse_y >= tooltip_y && mouse_y < tooltip_y + tooltip_height)
                {
                    tooltip_opts_t *option = tooltip_opts_select();

                    if (option == NULL)
                    {
                        return GAME_INPUT_TICK;
                    }

                    switch (option->type)
                    {
                    case TOOLTIP_OPT_MOVE:
                        automove_set(tooltip_tile_x, tooltip_tile_y, false, false);

                        break;

                    case TOOLTIP_OPT_ATTACK:
                        automove_attack = true;
                        automove_take_items = false;

                        automove_set(tooltip_tile_x, tooltip_tile_y, true, false);

                        break;

                    case TOOLTIP_OPT_TAKE:
                        automove_attack = false;
                        automove_take_items = true;

                        automove_set(tooltip_tile_x, tooltip_tile_y, false, true);

                        break;
                    }

                    tooltip_hide();
                }
                else
                {
                    tooltip_hide();
                }

                return GAME_INPUT_DRAW;
            }

            automove_set(mouse_tile_x, mouse_tile_y, true, true);

            return GAME_INPUT_DRAW;
        }
        else if (mouse.rbutton)
        {
            if (mouse_tile_x >= view_x && mouse_tile_x < view_x + view_width && mouse_tile_y >= view_y && mouse_tile_y < view_y + view_height)
            {
                tooltip_hide();

                tooltip_show(mouse_tile_x, mouse_tile_y);

                tooltip_opts_clear();

                tile_t *tile = &player->map->tiles[tooltip_tile_x][tooltip_tile_y];

                tooltip_opts_add(tile->actor == NULL ? "Move" : "Follow", TOOLTIP_OPT_MOVE);
                if (tile->actor != NULL)
                {
                    tooltip_opts_add("Attack", TOOLTIP_OPT_ATTACK);
                }
                if (TCOD_list_peek(tile->items))
                {
                    tooltip_opts_add("Take", TOOLTIP_OPT_TAKE);
                }

                return GAME_INPUT_DRAW;
            }

            return GAME_INPUT_TICK;
        }
        else if (mouse.wheel_down)
        {
            panel_content_scroll_down();

            return GAME_INPUT_DRAW;
        }
        else if (mouse.wheel_up)
        {
            panel_content_scroll_up();

            return GAME_INPUT_DRAW;
        }

        return GAME_INPUT_TICK;

    case TCOD_EVENT_KEY_PRESS:
        switch (key.vk)
        {
        case TCODK_ESCAPE:
            return GAME_INPUT_QUIT;

        case TCODK_PAGEDOWN:
            panel_content_scroll_down();

            return GAME_INPUT_DRAW;

        case TCODK_PAGEUP:
            panel_content_scroll_up();

            return GAME_INPUT_DRAW;

        case TCODK_CHAR:
            switch (key.c)
            {
            case ',':
                if (key.shift)
                {
                    tile_t *tile = &player->map->tiles[player->x][player->y];
                    if (tile->type != TILE_TYPE_STAIR_UP)
                    {
                        // return GAME_INPUT_TICK;
                    }

                    if (current_map_index <= 0)
                    {
                        return GAME_INPUT_QUIT;
                    }

                    current_map_index--;

                    map_t *new_map = TCOD_list_get(maps, current_map_index);

                    TCOD_list_remove(player->map->actors, player);
                    TCOD_list_push(new_map->actors, player);

                    player->map->tiles[player->x][player->y].actor = NULL;
                    new_map->tiles[new_map->stair_down_x][new_map->stair_down_y].actor = player;

                    player->map = new_map;
                    player->x = new_map->stair_down_x;
                    player->y = new_map->stair_down_y;

                    return GAME_INPUT_TURN;
                }

                return GAME_INPUT_TICK;

            case '.':
                if (key.shift)
                {
                    tile_t *tile = &player->map->tiles[player->x][player->y];
                    if (tile->type != TILE_TYPE_STAIR_DOWN)
                    {
                        // return GAME_INPUT_TICK;
                    }

                    current_map_index++;

                    map_t *new_map = TCOD_list_size(maps) == current_map_index
                                         ? map_create()
                                         : TCOD_list_get(maps, current_map_index);

                    TCOD_list_remove(player->map->actors, player);
                    TCOD_list_push(new_map->actors, player);

                    player->map->tiles[player->x][player->y].actor = NULL;
                    new_map->tiles[new_map->stair_up_x][new_map->stair_up_y].actor = player;

                    player->map = new_map;
                    player->x = new_map->stair_up_x;
                    player->y = new_map->stair_up_y;

                    return GAME_INPUT_TURN;
                }

                return GAME_INPUT_TICK;

            case 'a':
                sfx = !sfx;

                return GAME_INPUT_DRAW;

            case 'b':
                return GAME_INPUT_TICK;

            case 'c':
                panel_toggle(CONTENT_CHARACTER);

                return GAME_INPUT_DRAW;

            case 'd':
                return GAME_INPUT_TICK;

            case 'e':
                return GAME_INPUT_TICK;

            case 'f':
                return GAME_INPUT_TICK;

            case 'g':
                return GAME_INPUT_TICK;

            case 'h':
                return GAME_INPUT_TICK;

            case 'i':
                panel_toggle(CONTENT_INVENTORY);

                return GAME_INPUT_DRAW;

            case 'j':
                return GAME_INPUT_TICK;

            case 'k':
                return GAME_INPUT_TICK;

            case 'l':
                if (key.lctrl)
                {
                    game_uninit();
                    game_init();
                    game_load();

                    return GAME_INPUT_DRAW;
                }

                msg_visible = !msg_visible;

                return GAME_INPUT_DRAW;

            case 'm':
                return GAME_INPUT_TICK;

            case 'n':
                return GAME_INPUT_TICK;

            case 'o':
                return GAME_INPUT_TICK;

            case 'p':
                return GAME_INPUT_TICK;

            case 'q':
                return GAME_INPUT_TICK;

            case 'r':
                game_uninit();
                game_init();
                game_new();

                return GAME_INPUT_DRAW;

            case 's':
                if (key.lctrl)
                {
                    game_save();
                }

                return GAME_INPUT_DRAW;

            case 't':
                player->torch = !player->torch;

                if (player->torch)
                {
                    player->fov_radius *= 2;
                }
                else
                {
                    player->fov_radius /= 2;
                }

                actor_calc_fov(player);

                return GAME_INPUT_DRAW;

            case 'u':
                return GAME_INPUT_TICK;

            case 'v':
                return GAME_INPUT_TICK;

            case 'w':
                return GAME_INPUT_TICK;

            case 'x':
                return GAME_INPUT_TICK;

            case 'y':
                torch_color = TCOD_color_RGB(TCOD_random_get_int(NULL, 0, 255), TCOD_random_get_int(NULL, 0, 255), TCOD_random_get_int(NULL, 0, 255));

                return GAME_INPUT_DRAW;

            case 'z':
                return GAME_INPUT_TICK;
            }

            return GAME_INPUT_TICK;

        case TCODK_KP1:
            automove_clear();

            tooltip_hide();

            return actor_move(player, player->x - 1, player->y + 1, true, true)
                       ? GAME_INPUT_TURN
                       : GAME_INPUT_DRAW;

        case TCODK_KP2:
        case TCODK_DOWN:
            automove_clear();

            tooltip_hide();

            return actor_move(player, player->x, player->y + 1, true, true)
                       ? GAME_INPUT_TURN
                       : GAME_INPUT_DRAW;

        case TCODK_KP3:
            automove_clear();

            tooltip_hide();

            return actor_move(player, player->x + 1, player->y + 1, true, true)
                       ? GAME_INPUT_TURN
                       : GAME_INPUT_DRAW;

        case TCODK_KP4:
        case TCODK_LEFT:
            automove_clear();

            tooltip_hide();

            return actor_move(player, player->x - 1, player->y, true, true)
                       ? GAME_INPUT_TURN
                       : GAME_INPUT_DRAW;

        case TCODK_KP5:
            automove_clear();

            tooltip_hide();

            return GAME_INPUT_TURN;

        case TCODK_KP6:
        case TCODK_RIGHT:
            automove_clear();

            tooltip_hide();

            return actor_move(player, player->x + 1, player->y, true, true)
                       ? GAME_INPUT_TURN
                       : GAME_INPUT_DRAW;

        case TCODK_KP7:
            automove_clear();

            tooltip_hide();

            return actor_move(player, player->x - 1, player->y - 1, true, true)
                       ? GAME_INPUT_TURN
                       : GAME_INPUT_DRAW;

        case TCODK_KP8:
        case TCODK_UP:
            automove_clear();

            tooltip_hide();

            return actor_move(player, player->x, player->y - 1, true, true)
                       ? GAME_INPUT_TURN
                       : GAME_INPUT_DRAW;

        case TCODK_KP9:
            automove_clear();

            tooltip_hide();

            return actor_move(player, player->x + 1, player->y - 1, true, true)
                       ? GAME_INPUT_TURN
                       : GAME_INPUT_DRAW;
        }

        return GAME_INPUT_TICK;
    }

    static float automove_timer = 0.0f;
    static bool automove_ready = true;

    if (automove_ready && automove)
    {
        int x = automove_actor == NULL
                    ? automove_x
                    : automove_actor->x;
        int y = automove_actor == NULL
                    ? automove_y
                    : automove_actor->y;

        if (actor_move_towards(player, x, y, automove_attack, automove_take_items))
        {
            automove_ready = false;

            return GAME_INPUT_TURN;
        }
        else
        {
            automove_clear();
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

    return GAME_INPUT_TICK;
}

static void automove_set(int x, int y, bool attack, bool take_items)
{
    tile_t *tile = &player->map->tiles[x][y];
    actor_t *actor = tile->actor;

    automove = true;
    automove_actor = actor;
    automove_x = x;
    automove_y = y;
    automove_attack = attack;
    automove_take_items = take_items;
}

static void automove_clear()
{
    automove = false;
    automove_actor = NULL;
    automove_x = -1;
    automove_y = -1;
    automove_attack = false;
    automove_take_items = false;
}

void input_uninit(void)
{
}