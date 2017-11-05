#include <libtcod.h>

#include "CMemLeak.h"
#include "game.h"
#include "system.h"

void game_initialize(void)
{
    maps = TCOD_list_new();

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
    tile_color_dark = TCOD_darkest_gray;
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

    // TODO: the lights break if this is moved to the top of this function
    // why?
    map_t *map = map_create();
    player = actor_create(map, map->stair_up_x, map->stair_up_y, '@', TCOD_white, 5);
    current_map_index = 0;

    msg_log("Hail, Player!", player->map, player->x, player->y);
}

game_input_t game_input(void)
{
    static int automove_x = -1;
    static int automove_y = -1;
    static actor_t *automove_actor = NULL;

    TCOD_key_t key;
    TCOD_mouse_t mouse;
    TCOD_event_t ev = TCOD_sys_check_for_event(TCOD_EVENT_ANY, &key, &mouse);

    switch (ev)
    {
    case TCOD_EVENT_MOUSE_PRESS:
        if (mouse.lbutton)
        {
            automove_x = -1;
            automove_y = -1;
            automove_actor = NULL;

            int mouse_x = mouse.cx + view_x;
            int mouse_y = mouse.cy + view_y;

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
        automove_x = -1;
        automove_y = -1;
        automove_actor = NULL;

        // TODO: find a place for this
        tile_t *tile_n = player->y - 1 > 0
                             ? &player->map->tiles[player->x][player->y - 1]
                             : NULL;
        bool walkable_n = tile_n == NULL
                              ? false
                              : tile_walkable[tile_n->type];
        tile_t *tile_e = player->x + 1 < MAP_WIDTH
                             ? &player->map->tiles[player->x + 1][player->y]
                             : NULL;
        bool walkable_e = tile_e == NULL
                              ? false
                              : tile_walkable[tile_e->type];
        tile_t *tile_s = player->y + 1 < MAP_HEIGHT
                             ? &player->map->tiles[player->x][player->y + 1]
                             : NULL;
        bool walkable_s = tile_s == NULL
                              ? false
                              : tile_walkable[tile_s->type];
        tile_t *tile_w = player->x - 1 > 0
                             ? &player->map->tiles[player->x - 1][player->y]
                             : NULL;
        bool walkable_w = tile_w == NULL
                              ? false
                              : tile_walkable[tile_w->type];

        switch (key.vk)
        {
        case TCODK_ESCAPE:
            return GAME_INPUT_QUIT;

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

                    player->x = new_map->stair_down_x;
                    player->y = new_map->stair_down_y;

                    player->map = new_map;

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

                    player->x = new_map->stair_up_x;
                    player->y = new_map->stair_up_y;

                    player->map = new_map;

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
                actor_pick_item(player, &player->map->tiles[player->x][player->y]);

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

                return GAME_INPUT_TICK;

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
            if (walkable_s || walkable_w)
            {
                return actor_move(player, player->x - 1, player->y + 1)
                           ? GAME_INPUT_TURN
                           : GAME_INPUT_DRAW;
            }

            return GAME_INPUT_TICK;

        case TCODK_KP2:
        case TCODK_DOWN:
            return actor_move(player, player->x, player->y + 1)
                       ? GAME_INPUT_TURN
                       : GAME_INPUT_DRAW;

        case TCODK_KP3:
            if (walkable_e || walkable_s)
            {
                return actor_move(player, player->x + 1, player->y + 1)
                           ? GAME_INPUT_TURN
                           : GAME_INPUT_DRAW;
            }

            return GAME_INPUT_TICK;

        case TCODK_KP4:
        case TCODK_LEFT:
            return actor_move(player, player->x - 1, player->y)
                       ? GAME_INPUT_TURN
                       : GAME_INPUT_DRAW;

        case TCODK_KP5:
            return GAME_INPUT_TURN;

        case TCODK_KP6:
        case TCODK_RIGHT:
            return actor_move(player, player->x + 1, player->y)
                       ? GAME_INPUT_TURN
                       : GAME_INPUT_DRAW;

        case TCODK_KP7:
            if (walkable_n || walkable_w)
            {
                return actor_move(player, player->x - 1, player->y - 1)
                           ? GAME_INPUT_TURN
                           : GAME_INPUT_DRAW;
            }

            return GAME_INPUT_TICK;

        case TCODK_KP8:
        case TCODK_UP:
            return actor_move(player, player->x, player->y - 1)
                       ? GAME_INPUT_TURN
                       : GAME_INPUT_DRAW;

        case TCODK_KP9:
            if (walkable_n || walkable_e)
            {
                return actor_move(player, player->x + 1, player->y - 1)
                           ? GAME_INPUT_TURN
                           : GAME_INPUT_DRAW;
            }

            return GAME_INPUT_TICK;
        }

        return GAME_INPUT_TICK;
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

            return GAME_INPUT_TURN;
        }
        else
        {
            automove_x = -1;
            automove_y = -1;
            automove_actor = NULL;
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

    TCOD_console_set_default_background(panel, background_color);
    TCOD_console_set_default_foreground(panel, foreground_color);
    TCOD_console_clear(panel);

    TCOD_console_set_default_background(msg, background_color);
    TCOD_console_set_default_foreground(msg, foreground_color);
    TCOD_console_clear(msg);

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

    map_draw_turn(player->map);

    if (msg_visible)
    {
        msg_draw();
    }

    if (panel_visible)
    {
        panel_draw();
    }
}

void game_draw_tick(void)
{
    map_draw_tick(player->map);

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
}