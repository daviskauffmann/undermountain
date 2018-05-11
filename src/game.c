#include <libtcod/libtcod.h>
#include <malloc.h>
#include <math.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include "config.h"
#include "actor.h"
#include "game.h"
#include "map.h"
#include "message.h"
#include "projectile.h"

static void fn_should_update(struct game *game);

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

    game->object_common.__placeholder = 0;

    game->object_info[OBJECT_DOOR_CLOSED].name = "Closed Door";
    game->object_info[OBJECT_DOOR_CLOSED].glyph = '+';
    game->object_info[OBJECT_DOOR_CLOSED].color = TCOD_white;
    game->object_info[OBJECT_DOOR_CLOSED].is_transparent = false;
    game->object_info[OBJECT_DOOR_CLOSED].is_walkable = true;
    game->object_info[OBJECT_DOOR_CLOSED].light_radius = -1;
    game->object_info[OBJECT_DOOR_CLOSED].light_color = TCOD_white;
    game->object_info[OBJECT_DOOR_CLOSED].light_flicker = false;

    game->object_info[OBJECT_DOOR_OPEN].name = "Open Door";
    game->object_info[OBJECT_DOOR_OPEN].glyph = '-';
    game->object_info[OBJECT_DOOR_OPEN].color = TCOD_white;
    game->object_info[OBJECT_DOOR_OPEN].is_transparent = true;
    game->object_info[OBJECT_DOOR_OPEN].is_walkable = true;
    game->object_info[OBJECT_DOOR_OPEN].light_radius = -1;
    game->object_info[OBJECT_DOOR_OPEN].light_color = TCOD_white;
    game->object_info[OBJECT_DOOR_OPEN].light_flicker = false;

    game->object_info[OBJECT_STAIR_DOWN].name = "Stair Down";
    game->object_info[OBJECT_STAIR_DOWN].glyph = '>';
    game->object_info[OBJECT_STAIR_DOWN].color = TCOD_white;
    game->object_info[OBJECT_STAIR_DOWN].is_transparent = true;
    game->object_info[OBJECT_STAIR_DOWN].is_walkable = true;
    game->object_info[OBJECT_STAIR_DOWN].light_radius = -1;
    game->object_info[OBJECT_STAIR_DOWN].light_color = TCOD_white;
    game->object_info[OBJECT_STAIR_DOWN].light_flicker = false;

    game->object_info[OBJECT_STAIR_UP].name = "Stair Up";
    game->object_info[OBJECT_STAIR_UP].glyph = '<';
    game->object_info[OBJECT_STAIR_UP].color = TCOD_white;
    game->object_info[OBJECT_STAIR_UP].is_transparent = true;
    game->object_info[OBJECT_STAIR_UP].is_walkable = true;
    game->object_info[OBJECT_STAIR_UP].light_radius = -1;
    game->object_info[OBJECT_STAIR_UP].light_color = TCOD_white;
    game->object_info[OBJECT_STAIR_UP].light_flicker = false;

    game->object_info[OBJECT_ALTAR].name = "Altar";
    game->object_info[OBJECT_ALTAR].glyph = '_';
    game->object_info[OBJECT_ALTAR].color = TCOD_white;
    game->object_info[OBJECT_ALTAR].is_transparent = false;
    game->object_info[OBJECT_ALTAR].is_walkable = false;
    game->object_info[OBJECT_ALTAR].light_radius = 3;
    game->object_info[OBJECT_ALTAR].light_color = TCOD_white;
    game->object_info[OBJECT_ALTAR].light_flicker = false;

    game->object_info[OBJECT_FOUNTAIN].name = "Fountain";
    game->object_info[OBJECT_FOUNTAIN].glyph = '{';
    game->object_info[OBJECT_FOUNTAIN].color = TCOD_blue;
    game->object_info[OBJECT_FOUNTAIN].is_transparent = false;
    game->object_info[OBJECT_FOUNTAIN].is_walkable = false;
    game->object_info[OBJECT_FOUNTAIN].light_radius = -1;
    game->object_info[OBJECT_FOUNTAIN].light_color = TCOD_white;
    game->object_info[OBJECT_FOUNTAIN].light_flicker = false;

    game->object_info[OBJECT_THRONE].name = "Throne";
    game->object_info[OBJECT_THRONE].glyph = '\\';
    game->object_info[OBJECT_THRONE].color = TCOD_yellow;
    game->object_info[OBJECT_THRONE].is_transparent = false;
    game->object_info[OBJECT_THRONE].is_walkable = false;
    game->object_info[OBJECT_THRONE].light_radius = -1;
    game->object_info[OBJECT_THRONE].light_color = TCOD_white;
    game->object_info[OBJECT_THRONE].light_flicker = false;

    game->object_info[OBJECT_TORCH].name = "Torch";
    game->object_info[OBJECT_TORCH].glyph = '*';
    game->object_info[OBJECT_TORCH].color = TCOD_light_amber;
    game->object_info[OBJECT_TORCH].is_transparent = false;
    game->object_info[OBJECT_TORCH].is_walkable = false;
    game->object_info[OBJECT_TORCH].light_radius = 5;
    game->object_info[OBJECT_TORCH].light_color = TCOD_light_amber;
    game->object_info[OBJECT_TORCH].light_flicker = true;

    game->actor_common.glow_radius = 5;
    game->actor_common.glow_color = TCOD_white;
    game->actor_common.torch_radius = 10;
    game->actor_common.torch_color = TCOD_light_amber;

    game->race_info[RACE_HUMAN].name = "Human";
    game->race_info[RACE_HUMAN].glyph = '@';
    game->race_info[RACE_HUMAN].energy_per_turn = 0.5f;

    game->race_info[RACE_ELF].name = "Elf";
    game->race_info[RACE_ELF].glyph = '@';
    game->race_info[RACE_ELF].energy_per_turn = 0.6f;

    game->race_info[RACE_DWARF].name = "Dwarf";
    game->race_info[RACE_DWARF].glyph = '@';
    game->race_info[RACE_DWARF].energy_per_turn = 0.4f;

    game->race_info[RACE_ORC].name = "Orc";
    game->race_info[RACE_ORC].glyph = 'o';
    game->race_info[RACE_ORC].energy_per_turn = 0.5f;

    game->race_info[RACE_BUGBEAR].name = "Bugbear";
    game->race_info[RACE_BUGBEAR].glyph = 'b';
    game->race_info[RACE_BUGBEAR].energy_per_turn = 0.5f;

    game->race_info[RACE_ZOMBIE].name = "Zombie";
    game->race_info[RACE_ZOMBIE].glyph = 'z';
    game->race_info[RACE_ZOMBIE].energy_per_turn = 0.3f;

    game->class_info[CLASS_FIGHTER].name = "Fighter";
    game->class_info[CLASS_FIGHTER].color = TCOD_white;

    game->class_info[CLASS_RANGER].name = "Ranger";
    game->class_info[CLASS_RANGER].color = TCOD_dark_green;

    game->class_info[CLASS_WIZARD].name = "Wizard";
    game->class_info[CLASS_WIZARD].color = TCOD_azure;

    game->class_info[CLASS_CLERIC].name = "Cleric";
    game->class_info[CLASS_CLERIC].color = TCOD_yellow;

    game->item_common.__placeholder = 0;

    game->item_info[ITEM_BOW].name = "Bow";
    game->item_info[ITEM_BOW].glyph = '}';
    game->item_info[ITEM_BOW].color = TCOD_white;

    game->item_info[ITEM_POTION].name = "Potion";
    game->item_info[ITEM_POTION].glyph = '!';
    game->item_info[ITEM_POTION].color = TCOD_white;

    game->item_info[ITEM_SCROLL].name = "Scroll";
    game->item_info[ITEM_SCROLL].glyph = '?';
    game->item_info[ITEM_SCROLL].color = TCOD_white;

    game->item_info[ITEM_SHIELD].name = "Shield";
    game->item_info[ITEM_SHIELD].glyph = ')';
    game->item_info[ITEM_SHIELD].color = TCOD_white;

    game->item_info[ITEM_SPEAR].name = "Spear";
    game->item_info[ITEM_SPEAR].glyph = '/';
    game->item_info[ITEM_SPEAR].color = TCOD_white;

    game->item_info[ITEM_SWORD].name = "Sword";
    game->item_info[ITEM_SWORD].glyph = '|';
    game->item_info[ITEM_SWORD].color = TCOD_white;

    for (int level = 0; level < NUM_MAPS; level++)
    {
        struct map *map = &game->maps[level];

        map_init(map, game, level);
    }

    game->player = NULL;
    game->action = ACTION_NONE;
    game->targeting = TARGETING_NONE;
    game->target_x = -1;
    game->target_y = -1;

    game->turn = 0;
    game->turn_available = true;
    game->should_update = true;
    game->should_restart = false;
    game->should_quit = false;
    game->game_over = false;

    game->messages = TCOD_list_new();

    game->current_panel = PANEL_CHARACTER;

    game->panel_status[PANEL_CHARACTER].scroll = 0;
    game->panel_status[PANEL_CHARACTER].current_index = 0;
    game->panel_status[PANEL_CHARACTER].max_index = 0;

    game->panel_status[PANEL_INVENTORY].scroll = 0;
    game->panel_status[PANEL_INVENTORY].current_index = 0;
    game->panel_status[PANEL_INVENTORY].max_index = 0;

    game->panel_status[PANEL_SPELLBOOK].scroll = 0;
    game->panel_status[PANEL_SPELLBOOK].current_index = 0;
    game->panel_status[PANEL_SPELLBOOK].max_index = 0;

    game->message_log_visible = true;
    game->panel_visible = false;

    return game;
}

void game_new(struct game *game)
{
    TCOD_sys_delete_file(SAVE_PATH);

    for (int level = 0; level < NUM_MAPS; level++)
    {
        struct map *map = &game->maps[level];

        map_generate_custom(map);
        // map_generate_bsp(map);
        map_populate(map);
    }

    {
        int level = 0;
        struct map *map = &game->maps[level];
        int x = map->stair_up_x;
        int y = map->stair_up_y;
        struct tile *tile = &map->tiles[x][y];

        game->player = actor_create(game, RACE_HUMAN, CLASS_FIGHTER, FACTION_GOOD, "Player", level, x, y);
        game->player->glow = false;

        TCOD_list_push(map->actors, game->player);
        TCOD_list_push(tile->actors, game->player);

        game_log(
            game,
            level,
            x,
            y,
            TCOD_white,
            "Hail, %s %s!",
            game->race_info[game->player->race].name,
            game->class_info[game->player->class].name);
    }
}

void game_save(struct game *game)
{
    (void)game;
}

void game_load(struct game *game)
{
    (void)game;
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

    if (game->turn_available)
    {
        switch (ev)
        {
        case TCOD_EVENT_KEY_PRESS:
        {
            switch (key.vk)
            {
            case TCODK_KP5:
            {
                game->should_update = true;
            }
            break;
            }
        }
        break;
        }

        if (!game->game_over)
        {
            switch (ev)
            {
            case TCOD_EVENT_KEY_PRESS:
            {
                switch (key.vk)
                {
                case TCODK_KP1:
                {
                    if (game->targeting != TARGETING_NONE)
                    {
                        game->target_x--;
                        game->target_y++;
                    }
                    else
                    {
                        int x = game->player->x - 1;
                        int y = game->player->y + 1;

                        if (game->action == ACTION_NONE)
                        {
                            if (key.lctrl)
                            {
                                game->should_update = actor_swing(game->player, x, y);
                            }
                            else
                            {
                                game->should_update = actor_move(game->player, x, y);
                            }
                        }
                        else
                        {
                            game->should_update = actor_interact(game->player, x, y, game->action);

                            game->action = ACTION_NONE;
                        }
                    }
                }
                break;
                case TCODK_KP2:
                {
                    if (game->panel_visible)
                    {
                        struct panel_status *panel_status = &game->panel_status[game->current_panel];

                        if (panel_status->current_index < panel_status->max_index)
                        {
                            panel_status->current_index++;
                        }
                    }
                    else
                    {
                        if (game->targeting != TARGETING_NONE)
                        {
                            game->target_x;
                            game->target_y++;
                        }
                        else
                        {
                            int x = game->player->x;
                            int y = game->player->y + 1;

                            if (game->action == ACTION_NONE)
                            {
                                if (key.lctrl)
                                {
                                    game->should_update = actor_swing(game->player, x, y);
                                }
                                else
                                {
                                    game->should_update = actor_move(game->player, x, y);
                                }
                            }
                            else
                            {
                                game->should_update = actor_interact(game->player, x, y, game->action);

                                game->action = ACTION_NONE;
                            }
                        }
                    }
                }
                break;
                case TCODK_KP3:
                {
                    if (game->targeting != TARGETING_NONE)
                    {
                        game->target_x++;
                        game->target_y++;
                    }
                    else
                    {
                        int x = game->player->x + 1;
                        int y = game->player->y + 1;

                        if (game->action == ACTION_NONE)
                        {
                            if (key.lctrl)
                            {
                                game->should_update = actor_swing(game->player, x, y);
                            }
                            else
                            {
                                game->should_update = actor_move(game->player, x, y);
                            }
                        }
                        else
                        {
                            game->should_update = actor_interact(game->player, x, y, game->action);

                            game->action = ACTION_NONE;
                        }
                    }
                }
                break;
                case TCODK_KP4:
                {
                    if (game->targeting != TARGETING_NONE)
                    {
                        game->target_x--;
                        game->target_y;
                    }
                    else
                    {
                        int x = game->player->x - 1;
                        int y = game->player->y;

                        if (game->action == ACTION_NONE)
                        {
                            if (key.lctrl)
                            {
                                game->should_update = actor_swing(game->player, x, y);
                            }
                            else
                            {
                                game->should_update = actor_move(game->player, x, y);
                            }
                        }
                        else
                        {
                            game->should_update = actor_interact(game->player, x, y, game->action);

                            game->action = ACTION_NONE;
                        }
                    }
                }
                break;
                case TCODK_KP6:
                {
                    if (game->targeting != TARGETING_NONE)
                    {
                        game->target_x++;
                        game->target_y;
                    }
                    else
                    {
                        int x = game->player->x + 1;
                        int y = game->player->y;

                        if (game->action == ACTION_NONE)
                        {
                            if (key.lctrl)
                            {
                                game->should_update = actor_swing(game->player, x, y);
                            }
                            else
                            {
                                game->should_update = actor_move(game->player, x, y);
                            }
                        }
                        else
                        {
                            game->should_update = actor_interact(game->player, x, y, game->action);

                            game->action = ACTION_NONE;
                        }
                    }
                }
                break;
                case TCODK_KP7:
                {
                    if (game->targeting != TARGETING_NONE)
                    {
                        game->target_x--;
                        game->target_y--;
                    }
                    else
                    {
                        int x = game->player->x - 1;
                        int y = game->player->y - 1;

                        if (game->action == ACTION_NONE)
                        {
                            if (key.lctrl)
                            {
                                game->should_update = actor_swing(game->player, x, y);
                            }
                            else
                            {
                                game->should_update = actor_move(game->player, x, y);
                            }
                        }
                        else
                        {
                            game->should_update = actor_interact(game->player, x, y, game->action);

                            game->action = ACTION_NONE;
                        }
                    }
                }
                break;
                case TCODK_KP8:
                {
                    if (game->panel_visible)
                    {
                        struct panel_status *panel_status = &game->panel_status[game->current_panel];

                        if (panel_status->current_index > 0)
                        {
                            panel_status->current_index--;
                        }
                    }
                    else
                    {
                        if (game->targeting != TARGETING_NONE)
                        {
                            game->target_x;
                            game->target_y--;
                        }
                        else
                        {
                            int x = game->player->x;
                            int y = game->player->y - 1;

                            if (game->action == ACTION_NONE)
                            {
                                if (key.lctrl)
                                {
                                    game->should_update = actor_swing(game->player, x, y);
                                }
                                else
                                {
                                    game->should_update = actor_move(game->player, x, y);
                                }
                            }
                            else
                            {
                                game->should_update = actor_interact(game->player, x, y, game->action);

                                game->action = ACTION_NONE;
                            }
                        }
                    }
                }
                break;
                case TCODK_KP9:
                {
                    if (game->targeting != TARGETING_NONE)
                    {
                        game->target_x++;
                        game->target_y--;
                    }
                    else
                    {
                        int x = game->player->x + 1;
                        int y = game->player->y - 1;

                        if (game->action == ACTION_NONE)
                        {
                            if (key.lctrl)
                            {
                                game->should_update = actor_swing(game->player, x, y);
                            }
                            else
                            {
                                game->should_update = actor_move(game->player, x, y);
                            }
                        }
                        else
                        {
                            game->should_update = actor_interact(game->player, x, y, game->action);

                            game->action = ACTION_NONE;
                        }
                    }
                }
                break;
                case TCODK_CHAR:
                {
                    switch (key.c)
                    {
                    case '<':
                    {
                        game->should_update = actor_ascend(game->player);
                    }
                    break;
                    case '>':
                    {
                        game->should_update = actor_descend(game->player);
                    }
                    break;
                    case 'b':
                    {
                        game_panel_toggle(game, PANEL_SPELLBOOK);
                    }
                    break;
                    case 'C':
                    {
                        game_panel_toggle(game, PANEL_CHARACTER);
                    }
                    break;
                    case 'c':
                    {
                        game->action = ACTION_CLOSE_DOOR;

                        game_log(
                            game,
                            game->player->level,
                            game->player->x,
                            game->player->y,
                            TCOD_white,
                            "Choose a direction");
                    }
                    break;
                    case 'd':
                    {
                        game->should_update = actor_drop(game->player);
                    }
                    break;
                    case 'f':
                    {
                        if (game->targeting == TARGETING_SHOOT)
                        {
                            actor_shoot(game->player, game->target_x, game->target_y, &fn_should_update, game);

                            game->targeting = TARGETING_NONE;
                        }
                        else
                        {
                            game->targeting = TARGETING_SHOOT;

                            bool target_found = false;

                            struct map *map = &game->maps[game->player->level];

                            for (void **iterator = TCOD_list_begin(map->actors); iterator != TCOD_list_end(map->actors); iterator++)
                            {
                                struct actor *actor = *iterator;

                                if (actor->dead)
                                {
                                    continue;
                                }

                                if (TCOD_map_is_in_fov(game->player->fov, actor->x, actor->y) &&
                                    actor->faction != game->player->faction)
                                {
                                    target_found = true;

                                    game->target_x = actor->x;
                                    game->target_y = actor->y;

                                    break;
                                }
                            }

                            if (!target_found)
                            {
                                game->target_x = game->player->x;
                                game->target_y = game->player->y;
                            }
                        }
                    }
                    break;
                    case 'g':
                    {
                        game->should_update = actor_grab(game->player, game->player->x, game->player->y);
                    }
                    break;
                    case 'i':
                    {
                        game_panel_toggle(game, PANEL_INVENTORY);
                    }
                    break;
                    case 'l':
                    {
                        if (game->targeting == TARGETING_LOOK)
                        {
                            game->targeting = TARGETING_NONE;
                        }
                        else
                        {
                            game->targeting = TARGETING_LOOK;

                            game->target_x = game->player->x;
                            game->target_y = game->player->y;
                        }
                    }
                    break;
                    case 'm':
                    {
                        game->message_log_visible = !game->message_log_visible;
                    }
                    break;
                    case 'o':
                    {
                        game->action = ACTION_OPEN_DOOR;

                        game_log(
                            game,
                            game->player->level,
                            game->player->x,
                            game->player->y,
                            TCOD_white,
                            "Choose a direction");
                    }
                    break;
                    case 's':
                    {
                        if (key.lctrl)
                        {
                            game_save(game);
                        }
                    }
                    break;
                    case 't':
                    {
                        game->should_update = true;

                        game->player->torch = !game->player->torch;
                    }
                    break;
                    case 'z':
                    {
                        // TODO: spells
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
    }
}

void game_update(struct game *game)
{
    game->turn_available = true;

    struct map *map = &game->maps[game->player->level];

    for (void **iterator = TCOD_list_begin(map->actors); iterator != TCOD_list_end(map->actors); iterator++)
    {
        struct actor *actor = *iterator;

        actor_update_flash(actor);
    }

    for (void **iterator = TCOD_list_begin(map->projectiles); iterator != TCOD_list_end(map->projectiles); iterator++)
    {
        struct projectile *projectile = *iterator;

        projectile_update(projectile);

        if (projectile->destroyed)
        {
            iterator = TCOD_list_remove_iterator(map->projectiles, iterator);

            projectile_destroy(projectile);
        }
    }

    if (game->should_update)
    {
        game->should_update = false;
        game->turn++;

        for (void **iterator = TCOD_list_begin(map->objects); iterator != TCOD_list_end(map->objects); iterator++)
        {
            struct object *object = *iterator;

            if (object->destroyed)
            {
                struct tile *tile = &map->tiles[object->x][object->y];

                TCOD_list_remove(tile->objects, object);
                iterator = TCOD_list_remove_iterator(map->objects, iterator);

                object_destroy(object);

                continue;
            }

            object_calc_light(object);
        }

        for (void **iterator = TCOD_list_begin(map->actors); iterator != TCOD_list_end(map->actors); iterator++)
        {
            struct actor *actor = *iterator;

            actor_calc_light(actor);
        }

        for (void **iterator = TCOD_list_begin(map->actors); iterator != TCOD_list_end(map->actors); iterator++)
        {
            struct actor *actor = *iterator;

            actor_calc_fov(actor);

            if (!actor->dead)
            {
                actor_ai(actor);
            }
        }
    }
}

void game_render(struct game *game)
{
    TCOD_console_set_default_background(NULL, TCOD_black);
    TCOD_console_set_default_foreground(NULL, TCOD_white);
    TCOD_console_clear(NULL);

    int message_log_x = 0;
    int message_log_height = console_height / 4;
    int message_log_y = console_height - message_log_height;
    int message_log_width = console_width;

    int panel_width = console_width / 2;
    int panel_x = console_width - panel_width;
    int panel_y = 0;
    int panel_height = console_height - (game->message_log_visible ? message_log_height : 0);

    int view_width = console_width - (game->panel_visible ? panel_width : 0);
    int view_height = console_height - (game->message_log_visible ? message_log_height : 0);
    int view_x = game->player->x - view_width / 2;
    int view_y = game->player->y - view_height / 2;

    // view_x = view_x < 0
    //              ? 0
    //              : view_x + view_width > MAP_WIDTH
    //                    ? MAP_WIDTH - view_width
    //                    : view_x;
    // view_y = view_y < 0
    //              ? 0
    //              : view_y + view_height > MAP_HEIGHT
    //                    ? MAP_HEIGHT - view_height
    //                    : view_y;

    game->panel_status[PANEL_INVENTORY].max_index = TCOD_list_size(game->player->items) - 1;

    struct map *map = &game->maps[game->player->level];

    {
        static TCOD_noise_t noise = NULL;
        static float noise_x = 0.0f;

        if (noise == NULL)
        {
            noise = TCOD_noise_new(1, TCOD_NOISE_DEFAULT_HURST, TCOD_NOISE_DEFAULT_LACUNARITY, NULL);
        }

        noise_x += 0.2f;
        float noise_dx = noise_x + 20.0f;
        float dx = TCOD_noise_get(noise, &noise_dx) * 0.5f;
        noise_dx += 30.0f;
        float dy = TCOD_noise_get(noise, &noise_dx) * 0.5f;
        float di = 0.2f * TCOD_noise_get(noise, &noise_x);

        for (int x = view_x; x < view_x + view_width; x++)
        {
            for (int y = view_y; y < view_y + view_height; y++)
            {
                if (map_is_inside(x, y))
                {
                    struct tile *tile = &map->tiles[x][y];
                    struct tile_info *tile_info = &game->tile_info[tile->type];

                    TCOD_color_t color = game->tile_common.shadow_color;

                    if (TCOD_map_is_in_fov(game->player->fov, x, y))
                    {
                        tile->seen = true;

                        for (void **iterator = TCOD_list_begin(map->actors); iterator != TCOD_list_end(map->actors); iterator++)
                        {
                            struct actor *actor = *iterator;

                            if (actor->glow_fov && TCOD_map_is_in_fov(actor->glow_fov, x, y))
                            {
                                float r2 = powf((float)game->actor_common.glow_radius, 2);
                                float d = powf((float)(x - actor->x), 2) + powf((float)(y - actor->y), 2);
                                float l = CLAMP(0.0f, 1.0f, (r2 - d) / r2);

                                color = TCOD_color_lerp(color, TCOD_color_lerp(tile_info->color, game->actor_common.glow_color, l), l);
                            }
                        }

                        for (void **iterator = TCOD_list_begin(map->objects); iterator != TCOD_list_end(map->objects); iterator++)
                        {
                            struct object *object = *iterator;

                            if (object->light_fov && TCOD_map_is_in_fov(object->light_fov, x, y))
                            {
                                float r2 = powf((float)game->object_info[object->type].light_radius, 2);
                                float d = powf((float)(x - object->x + (game->object_info[object->type].light_flicker ? dx : 0)), 2) + powf((float)(y - object->y + (game->object_info[object->type].light_flicker ? dy : 0)), 2);
                                float l = CLAMP(0.0f, 1.0f, (r2 - d) / r2 + (game->object_info[object->type].light_flicker ? di : 0));

                                color = TCOD_color_lerp(color, TCOD_color_lerp(tile_info->color, game->object_info[object->type].light_color, l), l);
                            }
                        }

                        for (void **iterator = TCOD_list_begin(map->actors); iterator != TCOD_list_end(map->actors); iterator++)
                        {
                            struct actor *actor = *iterator;

                            if (actor->torch_fov && TCOD_map_is_in_fov(actor->torch_fov, x, y))
                            {
                                float r2 = powf((float)game->actor_common.torch_radius, 2);
                                float d = powf(x - actor->x + dx, 2) + powf(y - actor->y + dy, 2);
                                float l = CLAMP(0.0f, 1.0f, (r2 - d) / r2 + di);

                                color = TCOD_color_lerp(color, TCOD_color_lerp(tile_info->color, game->actor_common.torch_color, l), l);
                            }
                        }
                    }

                    if (tile->seen)
                    {
                        TCOD_console_set_char_foreground(NULL, x - view_x, y - view_y, color);
                        TCOD_console_set_char(NULL, x - view_x, y - view_y, tile_info->glyph);
                    }
                }
            }
        }

        for (void **iterator = TCOD_list_begin(map->actors); iterator != TCOD_list_end(map->actors); iterator++)
        {
            struct actor *actor = *iterator;

            if (actor->dead && TCOD_map_is_in_fov(game->player->fov, actor->x, actor->y))
            {
                TCOD_console_set_char_foreground(NULL, actor->x - view_x, actor->y - view_y, TCOD_dark_red);
                TCOD_console_set_char(NULL, actor->x - view_x, actor->y - view_y, '%');
            }
        }

        for (void **iterator = TCOD_list_begin(map->objects); iterator != TCOD_list_end(map->objects); iterator++)
        {
            struct object *object = *iterator;

            if (TCOD_map_is_in_fov(game->player->fov, object->x, object->y))
            {
                TCOD_console_set_char_foreground(NULL, object->x - view_x, object->y - view_y, game->object_info[object->type].color);
                TCOD_console_set_char(NULL, object->x - view_x, object->y - view_y, game->object_info[object->type].glyph);
            }
        }

        for (void **iterator = TCOD_list_begin(map->items); iterator != TCOD_list_end(map->items); iterator++)
        {
            struct item *item = *iterator;

            if (TCOD_map_is_in_fov(game->player->fov, item->x, item->y))
            {
                TCOD_console_set_char_foreground(NULL, item->x - view_x, item->y - view_y, game->item_info[item->type].color);
                TCOD_console_set_char(NULL, item->x - view_x, item->y - view_y, game->item_info[item->type].glyph);
            }
        }

        for (void **iterator = TCOD_list_begin(map->projectiles); iterator != TCOD_list_end(map->projectiles); iterator++)
        {
            struct projectile *projectile = *iterator;

            int x = (int)projectile->x;
            int y = (int)projectile->y;

            if (TCOD_map_is_in_fov(game->player->fov, x, y))
            {
                TCOD_console_set_char_foreground(NULL, x - view_x, y - view_y, TCOD_white);
                TCOD_console_set_char(NULL, x - view_x, y - view_y, projectile->glyph);
            }
        }

        for (void **iterator = TCOD_list_begin(map->actors); iterator != TCOD_list_end(map->actors); iterator++)
        {
            struct actor *actor = *iterator;

            if (!actor->dead && TCOD_map_is_in_fov(game->player->fov, actor->x, actor->y))
            {
                TCOD_color_t color = game->class_info[actor->class].color;

                if (actor->flash_fade > 0)
                {
                    color = TCOD_color_lerp(color, actor->flash_color, actor->flash_fade);
                }

                TCOD_console_set_char_foreground(NULL, actor->x - view_x, actor->y - view_y, color);
                TCOD_console_set_char(NULL, actor->x - view_x, actor->y - view_y, game->race_info[actor->race].glyph);
            }
        }
    }

    if (game->targeting != TARGETING_NONE)
    {
        TCOD_console_set_char_foreground(NULL, game->target_x - view_x, game->target_y - view_y, TCOD_red);
        TCOD_console_set_char(NULL, game->target_x - view_x, game->target_y - view_y, 'X');

        switch (game->targeting)
        {
        case TARGETING_LOOK:
        {
            struct tile *tile = &map->tiles[game->target_x][game->target_y];

            if (TCOD_map_is_in_fov(game->player->fov, game->target_x, game->target_y))
            {
                struct object *object = TCOD_list_peek(tile->objects);
                struct actor *actor = TCOD_list_peek(tile->actors);
                struct item *item = TCOD_list_peek(tile->items);

                if (object)
                {
                    TCOD_console_print_ex(NULL, console_width / 2, message_log_y - 2, TCOD_BKGND_NONE, TCOD_CENTER, game->object_info[object->type].name);

                    break;
                }

                if (actor)
                {
                    TCOD_console_print_ex(NULL, console_width / 2, message_log_y - 2, TCOD_BKGND_NONE, TCOD_CENTER, "%s %s", game->race_info[actor->race].name, game->class_info[actor->class].name);

                    break;
                }

                if (item)
                {
                    TCOD_console_print_ex(NULL, console_width / 2, message_log_y - 2, TCOD_BKGND_NONE, TCOD_CENTER, game->item_info[item->type].name);

                    break;
                }

                TCOD_console_print_ex(NULL, console_width / 2, message_log_y - 2, TCOD_BKGND_NONE, TCOD_CENTER, game->tile_info[tile->type].name);
            }
            else
            {
                if (tile->seen)
                {
                    TCOD_console_print_ex(NULL, console_width / 2, message_log_y - 2, TCOD_BKGND_NONE, TCOD_CENTER, "%s (known)", game->tile_info[tile->type].name);
                }
                else
                {
                    TCOD_console_print_ex(NULL, console_width / 2, message_log_y - 2, TCOD_BKGND_NONE, TCOD_CENTER, "Unknown");
                }
            }
            break;
        }
        }
    }

    if (game->message_log_visible)
    {
        static TCOD_console_t message_log = NULL;

        if (message_log == NULL)
        {
            message_log = TCOD_console_new(console_width, console_height);
        }

        TCOD_console_set_default_background(message_log, TCOD_black);
        TCOD_console_set_default_foreground(message_log, TCOD_white);
        TCOD_console_clear(message_log);

        int y = 1;
        for (void **i = TCOD_list_begin(game->messages); i != TCOD_list_end(game->messages); i++)
        {
            struct message *message = *i;

            TCOD_console_set_default_foreground(message_log, message->color);
            TCOD_console_print(message_log, 1, y, message->text);

            y++;
        }

        TCOD_console_set_default_foreground(message_log, TCOD_white);
        TCOD_console_print_frame(message_log, 0, 0, message_log_width, message_log_height, false, TCOD_BKGND_SET, "Log");

        TCOD_console_blit(message_log, 0, 0, message_log_width, message_log_height, NULL, message_log_x, message_log_y, 1, 1);
    }

    if (game->panel_visible)
    {
        static TCOD_console_t panel = NULL;

        if (panel == NULL)
        {
            panel = TCOD_console_new(console_width, console_height);
        }

        TCOD_console_set_default_background(panel, TCOD_black);
        TCOD_console_set_default_foreground(panel, TCOD_white);
        TCOD_console_clear(panel);

        struct panel_status *panel_status = &game->panel_status[game->current_panel];

        switch (game->current_panel)
        {
        case PANEL_CHARACTER:
        {
            TCOD_console_print(panel, 1, 1 - panel_status->scroll, "HP: 15 / 20");
            TCOD_console_print(panel, 1, 2 - panel_status->scroll, "MP:  7 / 16");

            TCOD_console_print(panel, 1, 4 - panel_status->scroll, "STR: 16");
            TCOD_console_print(panel, 1, 5 - panel_status->scroll, "DEX: 14");
            TCOD_console_print(panel, 1, 6 - panel_status->scroll, "CON: 12");
            TCOD_console_print(panel, 1, 7 - panel_status->scroll, "INT: 10");
            TCOD_console_print(panel, 1, 8 - panel_status->scroll, "WIS: 8");
            TCOD_console_print(panel, 1, 9 - panel_status->scroll, "CHA: 10");

            TCOD_console_print(panel, 1, 11 - panel_status->scroll, "R-Hand: Sword");
            TCOD_console_print(panel, 1, 12 - panel_status->scroll, "L-Hand: Shield");
            TCOD_console_print(panel, 1, 13 - panel_status->scroll, "Head  : Helm");
            TCOD_console_print(panel, 1, 14 - panel_status->scroll, "Chest : Cuirass");
            TCOD_console_print(panel, 1, 15 - panel_status->scroll, "Legs  : Greaves");
            TCOD_console_print(panel, 1, 16 - panel_status->scroll, "Feet  : Boots");

            TCOD_console_set_default_foreground(panel, TCOD_white);
            TCOD_console_print_frame(panel, 0, 0, panel_width, panel_height, false, TCOD_BKGND_SET, "Character");
        }
        break;
        case PANEL_INVENTORY:
        {
            int i = 0;
            int y = 1;
            for (void **iterator = TCOD_list_begin(game->player->items); iterator != TCOD_list_end(game->player->items); iterator++)
            {
                struct item *item = *iterator;

                TCOD_color_t color = panel_status->current_index == i ? TCOD_yellow : game->item_info[item->type].color;

                TCOD_console_set_default_foreground(panel, color);
                TCOD_console_print(panel, 1, y - panel_status->scroll, game->item_info[item->type].name);

                i++;
                y++;
            }

            TCOD_console_set_default_foreground(panel, TCOD_white);
            TCOD_console_print_frame(panel, 0, 0, panel_width, panel_height, false, TCOD_BKGND_SET, "Inventory");
        }
        break;
        case PANEL_SPELLBOOK:
        {
            TCOD_console_set_default_foreground(panel, TCOD_white);
            TCOD_console_print_frame(panel, 0, 0, panel_width, panel_height, false, TCOD_BKGND_SET, "Spellbook");
        }
        break;
        }

        TCOD_console_blit(panel, 0, 0, panel_width, panel_height, NULL, panel_x, panel_y, 1, 1);
    }

    TCOD_console_print(NULL, 0, 0, "Turn: %d", game->turn);
    TCOD_console_print(NULL, 0, 1, "Depth: %d", game->player->level);

    TCOD_console_flush();
}

void game_log(struct game *game, int level, int x, int y, TCOD_color_t color, char *fmt, ...)
{
    if (level != game->player->level ||
        !game->player->fov ||
        !TCOD_map_is_in_fov(game->player->fov, x, y))
    {
        return;
    }

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

void game_panel_toggle(struct game *game, enum panel_type panel_type)
{
    if (game->panel_visible)
    {
        if (game->current_panel == panel_type)
        {
            game->panel_visible = false;
        }
        else
        {
            game->current_panel = panel_type;
        }
    }
    else
    {
        game->current_panel = panel_type;
        game->panel_visible = true;
    }
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

    free(game);
}

static void fn_should_update(struct game *game)
{
    game->should_update = true;
}
