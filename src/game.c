#include <libtcod/libtcod.h>
#include <malloc.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include "game.h"
#include "map.h"
#include "message.h"
#include "window.h"

struct game *game_create(void)
{
    struct game *game = malloc(sizeof(struct game));

    game->tile_common.shadow_color = TCOD_color_RGB(16, 16, 32);

    game->tile_info[TILE_EMPTY].name = "Empty";
    game->tile_info[TILE_EMPTY].glyph = ' ';
    game->tile_info[TILE_EMPTY].color = TCOD_white;
    game->tile_info[TILE_EMPTY].is_transparent = true;
    game->tile_info[TILE_EMPTY].is_walkable = true;

    game->tile_info[TILE_FLOOR].name = "Floor";
    game->tile_info[TILE_FLOOR].glyph = '.';
    game->tile_info[TILE_FLOOR].color = TCOD_white;
    game->tile_info[TILE_FLOOR].is_transparent = true;
    game->tile_info[TILE_FLOOR].is_walkable = true;

    game->tile_info[TILE_WALL].name = "Wall";
    game->tile_info[TILE_WALL].glyph = '#';
    game->tile_info[TILE_WALL].color = TCOD_white;
    game->tile_info[TILE_WALL].is_transparent = false;
    game->tile_info[TILE_WALL].is_walkable = false;

    game->tile_info[TILE_DOOR_CLOSED].name = "Closed Door";
    game->tile_info[TILE_DOOR_CLOSED].glyph = '+';
    game->tile_info[TILE_DOOR_CLOSED].color = TCOD_white;
    game->tile_info[TILE_DOOR_CLOSED].is_transparent = false;
    game->tile_info[TILE_DOOR_CLOSED].is_walkable = false;

    game->tile_info[TILE_DOOR_OPEN].name = "Open Door";
    game->tile_info[TILE_DOOR_OPEN].glyph = '-';
    game->tile_info[TILE_DOOR_OPEN].color = TCOD_white;
    game->tile_info[TILE_DOOR_OPEN].is_transparent = true;
    game->tile_info[TILE_DOOR_OPEN].is_walkable = true;

    game->tile_info[TILE_STAIR_DOWN].name = "Stair Down";
    game->tile_info[TILE_STAIR_DOWN].glyph = '>';
    game->tile_info[TILE_STAIR_DOWN].color = TCOD_white;
    game->tile_info[TILE_STAIR_DOWN].is_transparent = true;
    game->tile_info[TILE_STAIR_DOWN].is_walkable = true;

    game->tile_info[TILE_STAIR_UP].name = "Stair Up";
    game->tile_info[TILE_STAIR_UP].glyph = '<';
    game->tile_info[TILE_STAIR_UP].color = TCOD_white;
    game->tile_info[TILE_STAIR_UP].is_transparent = true;
    game->tile_info[TILE_STAIR_UP].is_walkable = true;

    game->object_common.__placeholder = 0;

    game->object_info[OBJECT_BRAZIER].name = "Brazier";
    game->object_info[OBJECT_BRAZIER].glyph = '*';
    game->object_info[OBJECT_BRAZIER].color = TCOD_white;
    game->object_info[OBJECT_BRAZIER].is_transparent = false;
    game->object_info[OBJECT_BRAZIER].is_walkable = false;
    game->object_info[OBJECT_BRAZIER].light_radius = 10;
    game->object_info[OBJECT_BRAZIER].light_color = TCOD_white;

    game->actor_common.torch_radius = 10;
    game->actor_common.torch_color = TCOD_light_amber;

    game->actor_info[ACTOR_PLAYER].name = "Player";
    game->actor_info[ACTOR_PLAYER].glyph = '@';
    game->actor_info[ACTOR_PLAYER].color = TCOD_white;

    game->actor_info[ACTOR_DOG].name = "Dog";
    game->actor_info[ACTOR_DOG].glyph = 'd';
    game->actor_info[ACTOR_DOG].color = TCOD_white;

    game->actor_info[ACTOR_ORC].name = "Orc";
    game->actor_info[ACTOR_ORC].glyph = 'o';
    game->actor_info[ACTOR_ORC].color = TCOD_white;

    game->actor_info[ACTOR_BUGBEAR].name = "Bugbear";
    game->actor_info[ACTOR_BUGBEAR].glyph = 'b';
    game->actor_info[ACTOR_BUGBEAR].color = TCOD_white;

    game->actor_info[ACTOR_JACKAL].name = "Jackal";
    game->actor_info[ACTOR_JACKAL].glyph = 'j';
    game->actor_info[ACTOR_JACKAL].color = TCOD_orange;

    game->actor_info[ACTOR_ZOMBIE].name = "Zombie";
    game->actor_info[ACTOR_ZOMBIE].glyph = 'z';
    game->actor_info[ACTOR_ZOMBIE].color = TCOD_green;

    game->item_common.__placeholder = 0;

    game->item_info[ITEM_SWORD].name = "Sword";
    game->item_info[ITEM_SWORD].glyph = '|';
    game->item_info[ITEM_SWORD].color = TCOD_white;

    for (int level = 0; level < NUM_MAPS; level++)
    {
        struct map *map = &game->maps[level];

        map_init(map, game, level);
    }

    game->player = NULL;

    game->turn = 0;
    game->turn_available = true;
    game->should_update = true;
    game->should_restart = false;
    game->should_quit = false;
    game->game_over = false;

    game->messages = TCOD_list_new();

    game->current_panel = PANEL_CHARACTER;

    game->panel_status[PANEL_CHARACTER].current = 0;
    game->panel_status[PANEL_CHARACTER].scroll = 0;

    game->panel_status[PANEL_INVENTORY].current = 0;
    game->panel_status[PANEL_INVENTORY].scroll = 0;

    game->message_log_visible = true;
    game->panel_visible = false;

    return game;
}

void game_new(struct game *game)
{
    TCOD_sys_delete_file("../saves/save.gz");

    for (int level = 0; level < NUM_MAPS; level++)
    {
        struct map *map = &game->maps[level];

        // map_generate_custom(map);
        map_generate_bsp(map);
        map_populate(map);
    }

    {
        int level = 0;
        struct map *map = &game->maps[level];
        int x = map->stair_up_x;
        int y = map->stair_up_y;
        struct tile *tile = &map->tiles[x + y * MAP_WIDTH];

        game->player = actor_create(ACTOR_PLAYER, game, level, x, y, 20, FACTION_GOOD);
        TCOD_list_push(map->actors, game->player);
        TCOD_list_push(tile->actors, game->player);
    }

    game_log(game, game->player->level, game->player->x, game->player->y, TCOD_white, "Hail, %s", game->actor_info[ACTOR_PLAYER].name);
}

void game_save(struct game *game)
{
}

void game_load(struct game *game)
{
}

void game_input(struct game *game)
{
    TCOD_key_t key;
    TCOD_mouse_t mouse;
    TCOD_event_t ev = TCOD_sys_check_for_event(TCOD_EVENT_ANY, &key, &mouse);

    switch (ev)
    {
    case TCOD_EVENT_KEY_PRESS:
    {
        switch (key.vk)
        {
        case TCODK_ESCAPE:
        {
            game->should_quit = true;
        }
        break;
        case TCODK_ENTER:
        {
            if (key.lalt)
            {
                fullscreen = !fullscreen;

                TCOD_console_set_fullscreen(fullscreen);
            }
        }
        break;
        case TCODK_CHAR:
        {
            switch (key.c)
            {
            case 'r':
            {
                game->should_restart = true;
            }
            break;
            }
        }
        break;
        }
    }
    break;
    }
}

void game_update(struct game *game)
{
}

void game_render(struct game *game)
{
}

void game_log(struct game *game, int level, int x, int y, TCOD_color_t color, char *fmt, ...)
{
    char buffer[128];

    va_list args;
    va_start(args, fmt);
    vsprintf_s(buffer, sizeof(buffer), fmt, args);
    va_end(args);

    char *line_begin = buffer;
    char *line_end;

    do
    {
        if (TCOD_list_size(game->messages) == (console_height / 4) - 2)
        {
            struct message *message = TCOD_list_get(game->messages, 0);

            TCOD_list_remove(game->messages, message);

            message_destroy(message);
        }

        line_end = strchr(line_begin, '\n');

        if (line_end)
        {
            *line_end = '\0';
        }

        struct message *message = message_create(line_begin, color);

        TCOD_list_push(game->messages, message);

        line_begin = line_end + 1;
    } while (line_end);
}

void game_destroy(struct game *game)
{
    for (int i = 0; i < NUM_MAPS; i++)
    {
        struct map *map = &game->maps[i];

        map_reset(map);
    }

    for (void **iterator = TCOD_list_begin(game->messages); iterator != TCOD_list_end(game->messages); iterator++)
    {
        struct message *message = *iterator;

        message_destroy(message);
    }

    TCOD_list_delete(game->messages);
}
