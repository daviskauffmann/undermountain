#include <libtcod.h>

#include "CMemLeak.h"
#include "game.h"
#include "system.h"

void game_initialize(void)
{
    maps = TCOD_list_new();
    player = NULL;
    current_map_index = -1;

    tile_glyph[TILE_TYPE_EMPTY] = ' ';
    tile_glyph[TILE_TYPE_FLOOR] = '.';
    tile_glyph[TILE_TYPE_WALL] = '#';
    tile_glyph[TILE_TYPE_STAIR_DOWN] = '>';
    tile_glyph[TILE_TYPE_STAIR_UP] = '<';

    tile_transparent[TILE_TYPE_EMPTY] = true;
    tile_transparent[TILE_TYPE_FLOOR] = true;
    tile_transparent[TILE_TYPE_WALL] = false;
    tile_transparent[TILE_TYPE_STAIR_DOWN] = true;
    tile_transparent[TILE_TYPE_STAIR_UP] = true;

    tile_walkable[TILE_TYPE_EMPTY] = true;
    tile_walkable[TILE_TYPE_FLOOR] = true;
    tile_walkable[TILE_TYPE_WALL] = false;
    tile_walkable[TILE_TYPE_STAIR_DOWN] = true;
    tile_walkable[TILE_TYPE_STAIR_UP] = true;

    background_color = TCOD_black;
    foreground_color = TCOD_white;
    tile_color_light = TCOD_white;
    tile_color_dark = TCOD_color_RGB(16, 16, 16);
    torch_color = TCOD_light_amber;

    sfx = true;

    msg = TCOD_console_new(screen_width, screen_height);
    msg_visible = true;
    messages = TCOD_list_new();

    panel = TCOD_console_new(screen_width, screen_height);
    panel_visible = false;
    content = CONTENT_CHARACTER;
    content_scroll[CONTENT_CHARACTER] = 0;
    content_scroll[CONTENT_INVENTORY] = 0;

    tooltip = TCOD_console_new(screen_width, screen_height);
    tooltip_visible = false;

    alert = TCOD_console_new(screen_width, screen_height);
    alert_visible = false;

    map_t *map = map_create();
    current_map_index = 0;

    player = actor_create(map, map->stair_up_x, map->stair_up_y, '@', TCOD_white, 5);

    TCOD_list_push(map->actors, player);
    map->tiles[player->x][player->y].actor = player;

    item_t *item = item_create_random();

    TCOD_list_push(player->items, item);

    msg_log("Hail, Player!", player->map, player->x, player->y);
}

game_input_t game_input(void)
{
    static bool automove = false;
    static int automove_x = -1;
    static int automove_y = -1;
    static actor_t *automove_actor = NULL;

    TCOD_key_t key;
    TCOD_mouse_t mouse;
    TCOD_event_t ev = TCOD_sys_check_for_event(TCOD_EVENT_ANY, &key, &mouse);

    switch (ev)
    {
    case TCOD_EVENT_MOUSE_PRESS:
        int mouse_x = mouse.cx + view_x;
        int mouse_y = mouse.cy + view_y;

        if (mouse.lbutton)
        {
            if (mouse_x >= view_x && mouse_x < view_x + view_width && mouse_y >= view_y && mouse_y < view_y + view_height)
            {
                if (tooltip_visible)
                {
                    if (mouse_x >= tooltip_tile_x && mouse_x < tooltip_tile_x + tooltip_width && mouse_y >= tooltip_tile_y && mouse_y < tooltip_tile_y + tooltip_height)
                    {
                        // we are inside the tooltip
                        return GAME_INPUT_DRAW;
                    }
                }

                tooltip_visible = false;

                tile_t *tile = &player->map->tiles[mouse_x][mouse_y];
                actor_t *actor = tile->actor;

                automove = true;
                automove_actor = actor;
                automove_x = mouse_x;
                automove_y = mouse_y;

                return GAME_INPUT_DRAW;
            }

            return GAME_INPUT_TICK;
        }
        else if (mouse.rbutton)
        {
            if (tooltip_visible)
            {
                tooltip_visible = false;
            }
            else
            {
                tooltip_visible = true;
                tooltip_tile_x = mouse_x;
                tooltip_tile_y = mouse_y;
            }

            return GAME_INPUT_DRAW;
        }
        else if (mouse.wheel_down)
        {
            if (content_scroll[content] + panel_height < content_height[content])
            {
                content_scroll[content]++;
            }

            return GAME_INPUT_DRAW;
        }
        else if (mouse.wheel_up)
        {
            if (content_scroll[content] > 0)
            {
                content_scroll[content]--;
            }

            return GAME_INPUT_DRAW;
        }

        return GAME_INPUT_TICK;

    case TCOD_EVENT_KEY_PRESS:
        switch (key.vk)
        {
        case TCODK_ESCAPE:
            return GAME_INPUT_QUIT;

        case TCODK_ENTER:
            if (alert_visible)
            {
                alert_visible = false;

                return GAME_INPUT_DRAW;
            }

            return GAME_INPUT_TICK;

        case TCODK_PAGEDOWN:
            if (content_scroll[content] + panel_height < content_height[content])
            {
                content_scroll[content]++;
            }

            return GAME_INPUT_DRAW;

        case TCODK_PAGEUP:
            if (content_scroll[content] > 0)
            {
                content_scroll[content]--;
            }

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
                if (panel_visible && content == CONTENT_CHARACTER)
                {
                    panel_visible = false;
                }
                else
                {
                    content = CONTENT_CHARACTER;

                    panel_visible = true;
                }

                return GAME_INPUT_DRAW;

            case 'd':
                return GAME_INPUT_TICK;

            case 'e':
                return GAME_INPUT_TICK;

            case 'f':
                return GAME_INPUT_TICK;

            case 'g':
                tile_t *tile = &player->map->tiles[player->x][player->y];

                if (TCOD_list_size(tile->items) == 0)
                {
                    return GAME_INPUT_TICK;
                }

                TCOD_list_push(player->items, TCOD_list_pop(tile->items));

                return GAME_INPUT_DRAW;

            case 'h':
                return GAME_INPUT_TICK;

            case 'i':
                if (panel_visible && content == CONTENT_INVENTORY)
                {
                    panel_visible = false;
                }
                else
                {
                    content = CONTENT_INVENTORY;

                    panel_visible = true;
                }

                return GAME_INPUT_DRAW;

            case 'j':
                return GAME_INPUT_TICK;

            case 'k':
                return GAME_INPUT_TICK;

            case 'l':
                if (key.lctrl)
                {
                    game_finalize();

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
                game_finalize();

                game_initialize();

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
            automove = false;

            tooltip_visible = false;

            return actor_move(player, player->x - 1, player->y + 1)
                       ? GAME_INPUT_TURN
                       : GAME_INPUT_DRAW;

        case TCODK_KP2:
        case TCODK_DOWN:
            automove = false;

            tooltip_visible = false;

            return actor_move(player, player->x, player->y + 1)
                       ? GAME_INPUT_TURN
                       : GAME_INPUT_DRAW;

        case TCODK_KP3:
            automove = false;

            tooltip_visible = false;

            return actor_move(player, player->x + 1, player->y + 1)
                       ? GAME_INPUT_TURN
                       : GAME_INPUT_DRAW;

        case TCODK_KP4:
        case TCODK_LEFT:
            automove = false;

            tooltip_visible = false;

            return actor_move(player, player->x - 1, player->y)
                       ? GAME_INPUT_TURN
                       : GAME_INPUT_DRAW;

        case TCODK_KP5:
            automove = false;

            tooltip_visible = false;

            return GAME_INPUT_TURN;

        case TCODK_KP6:
        case TCODK_RIGHT:
            automove = false;

            tooltip_visible = false;

            return actor_move(player, player->x + 1, player->y)
                       ? GAME_INPUT_TURN
                       : GAME_INPUT_DRAW;

        case TCODK_KP7:
            automove = false;

            tooltip_visible = false;

            return actor_move(player, player->x - 1, player->y - 1)
                       ? GAME_INPUT_TURN
                       : GAME_INPUT_DRAW;

        case TCODK_KP8:
        case TCODK_UP:
            automove = false;

            tooltip_visible = false;

            return actor_move(player, player->x, player->y - 1)
                       ? GAME_INPUT_TURN
                       : GAME_INPUT_DRAW;

        case TCODK_KP9:
            automove = false;

            tooltip_visible = false;

            return actor_move(player, player->x + 1, player->y - 1)
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

        if (actor_move_towards(player, x, y))
        {
            automove_ready = false;

            return GAME_INPUT_TURN;
        }
        else
        {
            automove = false;
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

void game_turn(void)
{
#if SIMULATE_ALL_MAPS
    for (void **i = TCOD_list_begin(maps); i != TCOD_list_end(maps); i++)
    {
        map_t *map = *i;

        map_turn(map);
    }
#else
    map_turn(player->map);
#endif
}

void game_tick(void)
{
#if SIMULATE_ALL_MAPS
    for (void **i = TCOD_list_begin(maps); i != TCOD_list_end(maps); i++)
    {
        map_t *map = *i;

        map_tick(map);
    }
#else
    map_tick(player->map);
#endif
}

void game_save(void)
{
}

void game_load(void)
{
}

void game_draw_turn(void)
{
    TCOD_console_set_default_background(NULL, background_color);
    TCOD_console_set_default_foreground(NULL, foreground_color);
    TCOD_console_clear(NULL);

    panel_width = screen_width / 2;
    panel_x = screen_width - panel_width;
    panel_y = 0;
    panel_height = screen_height;

    msg_x = 0;
    msg_height = screen_height / 4;
    msg_y = screen_height - msg_height;
    msg_width = screen_width - (panel_visible ? panel_width : 0);

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

    // TODO: keep this in the view
    tooltip_width = 20;
    tooltip_height = 6;
    tooltip_x = tooltip_tile_x - view_x;
    tooltip_y = tooltip_tile_y - view_y;

    alert_width = 20;
    alert_height = 20;
    alert_x = (screen_width / 2) - (alert_width / 2);
    alert_y = (screen_height / 2) - (alert_height / 2);

    map_draw_turn(player->map);

    msg_draw_turn();

    panel_draw_turn();

    tooltip_draw_turn();

    alert_draw_turn();
}

void game_draw_tick(void)
{
    map_draw_tick(player->map);

    msg_draw_tick();

    panel_draw_tick();

    tooltip_draw_tick();

    alert_draw_tick();

    TCOD_console_flush();
}

void game_finalize(void)
{
    for (void **i = TCOD_list_begin(maps); i != TCOD_list_end(maps); i++)
    {
        map_t *map = *i;

        map_destroy(map);
    }

    TCOD_list_delete(maps);

    TCOD_console_delete(msg);
    TCOD_list_delete(messages);

    TCOD_console_delete(panel);

    TCOD_console_delete(tooltip);

    TCOD_console_delete(alert);
}