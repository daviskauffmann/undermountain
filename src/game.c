#include <libtcod/libtcod.h>
#include <malloc.h>
#include <math.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include "actor.h"
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

    game->actor_common.glow_radius = 5;
    game->actor_common.glow_color = TCOD_white;
    game->actor_common.torch_radius = 10;
    game->actor_common.torch_color = TCOD_light_amber;

    game->race_info[RACE_HUMAN].name = "Human";
    game->race_info[RACE_HUMAN].glyph = '@';

    game->race_info[RACE_ELF].name = "Elf";
    game->race_info[RACE_ELF].glyph = '@';

    game->race_info[RACE_DWARF].name = "Dwarf";
    game->race_info[RACE_DWARF].glyph = '@';

    game->race_info[RACE_DOG].name = "Dog";
    game->race_info[RACE_DOG].glyph = 'd';

    game->race_info[RACE_ORC].name = "Orc";
    game->race_info[RACE_ORC].glyph = 'o';

    game->race_info[RACE_BUGBEAR].name = "Bugbear";
    game->race_info[RACE_BUGBEAR].glyph = 'b';

    game->race_info[RACE_ZOMBIE].name = "Zombie";
    game->race_info[RACE_ZOMBIE].glyph = 'z';

    game->class_info[CLASS_FIGHTER].name = "Fighter";
    game->class_info[CLASS_FIGHTER].color = TCOD_white;

    game->class_info[CLASS_RANGER].name = "Ranger";
    game->class_info[CLASS_RANGER].color = TCOD_dark_green;

    game->class_info[CLASS_WIZARD].name = "Wizard";
    game->class_info[CLASS_WIZARD].color = TCOD_azure;

    game->class_info[CLASS_CLERIC].name = "Cleric";
    game->class_info[CLASS_CLERIC].color = TCOD_yellow;

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
        struct tile *tile = &map->tiles[x][y];

        game->player = actor_create(game, RACE_HUMAN, CLASS_FIGHTER, FACTION_GOOD, level, x, y);
        game->player->glow = true;

        TCOD_list_push(map->actors, game->player);
        TCOD_list_push(tile->actors, game->player);
    }

    game_log(game, game->player->level, game->player->x, game->player->y, TCOD_white, "Hail, %s", "Player");
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
                static enum action action = ACTION_NONE;

                switch (key.vk)
                {
                case TCODK_KP1:
                {
                    if (game->targeting)
                    {
                        game->target_x--;
                        game->target_y++;
                    }
                    else
                    {
                        int x = game->player->x - 1;
                        int y = game->player->y + 1;

                        if (action == ACTION_NONE)
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
                            game->should_update = actor_interact(game->player, x, y, action);

                            action = ACTION_NONE;
                        }
                    }
                }
                break;
                case TCODK_KP2:
                {
                    if (game->targeting)
                    {
                        game->target_x;
                        game->target_y++;
                    }
                    else
                    {
                        int x = game->player->x;
                        int y = game->player->y + 1;

                        if (action == ACTION_NONE)
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
                            game->should_update = actor_interact(game->player, x, y, action);

                            action = ACTION_NONE;
                        }
                    }
                }
                break;
                case TCODK_KP3:
                {
                    if (game->targeting)
                    {
                        game->target_x++;
                        game->target_y++;
                    }
                    else
                    {
                        int x = game->player->x + 1;
                        int y = game->player->y + 1;

                        if (action == ACTION_NONE)
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
                            game->should_update = actor_interact(game->player, x, y, action);

                            action = ACTION_NONE;
                        }
                    }
                }
                break;
                case TCODK_KP4:
                {
                    if (game->targeting)
                    {
                        game->target_x--;
                        game->target_y;
                    }
                    else
                    {
                        int x = game->player->x - 1;
                        int y = game->player->y;

                        if (action == ACTION_NONE)
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
                            game->should_update = actor_interact(game->player, x, y, action);

                            action = ACTION_NONE;
                        }
                    }
                }
                break;
                case TCODK_KP6:
                {
                    if (game->targeting)
                    {
                        game->target_x++;
                        game->target_y;
                    }
                    else
                    {
                        int x = game->player->x + 1;
                        int y = game->player->y;

                        if (action == ACTION_NONE)
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
                            game->should_update = actor_interact(game->player, x, y, action);

                            action = ACTION_NONE;
                        }
                    }
                }
                break;
                case TCODK_KP7:
                {
                    if (game->targeting)
                    {
                        game->target_x--;
                        game->target_y--;
                    }
                    else
                    {
                        int x = game->player->x - 1;
                        int y = game->player->y - 1;

                        if (action == ACTION_NONE)
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
                            game->should_update = actor_interact(game->player, x, y, action);

                            action = ACTION_NONE;
                        }
                    }
                }
                break;
                case TCODK_KP8:
                {
                    if (game->targeting)
                    {
                        game->target_x;
                        game->target_y--;
                    }
                    else
                    {
                        int x = game->player->x;
                        int y = game->player->y - 1;

                        if (action == ACTION_NONE)
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
                            game->should_update = actor_interact(game->player, x, y, action);

                            action = ACTION_NONE;
                        }
                    }
                }
                break;
                case TCODK_KP9:
                {
                    if (game->targeting)
                    {
                        game->target_x++;
                        game->target_y--;
                    }
                    else
                    {
                        int x = game->player->x + 1;
                        int y = game->player->y - 1;

                        if (action == ACTION_NONE)
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
                            game->should_update = actor_interact(game->player, x, y, action);

                            action = ACTION_NONE;
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
                    case 'C':
                    {
                        game->panel_visible = !game->panel_visible;
                    }
                    break;
                    case 'c':
                    {
                        action = ACTION_CLOSE_DOOR;

                        game_log(game, game->player->level, game->player->x, game->player->y, TCOD_white, "Choose a direction");
                    }
                    break;
                    case 'f':
                    {
                        // position_t *player_position = (position_t *)component_get(game->player, COMPONENT_POSITION);

                        // if (player_position)
                        // {
                        //     targeting_t *player_targeting = (targeting_t *)component_get(game->player, COMPONENT_TARGETING);

                        //     if (player_targeting && player_targeting->type == TARGETING_SHOOT)
                        //     {
                        //         entity_shoot(game->player, player_targeting->x, player_targeting->y, &fn_should_update, game);

                        //         component_remove(game->player, COMPONENT_TARGETING);
                        //     }
                        //     else
                        //     {
                        //         player_targeting = (targeting_t *)component_add(game->player, COMPONENT_TARGETING);
                        //         player_targeting->type = TARGETING_SHOOT;

                        //         bool target_found = false;

                        //         {
                        //             alignment_t *player_alignment = (alignment_t *)component_get(game->player, COMPONENT_ALIGNMENT);
                        //             fov_t *player_fov = (fov_t *)component_get(game->player, COMPONENT_FOV);

                        //             if (player_alignment && player_fov)
                        //             {
                        //                 for (void **iterator = TCOD_list_begin(game->maps[player_position->level].entities); iterator != TCOD_list_end(game->maps[player_position->level].entities); iterator++)
                        //                 {
                        //                     entity_t *other = *iterator;

                        //                     alignment_t *other_alignment = (alignment_t *)component_get(other, COMPONENT_ALIGNMENT);
                        //                     position_t *other_position = (position_t *)component_get(other, COMPONENT_POSITION);

                        //                     if (other_alignment && other_position)
                        //                     {
                        //                         if (TCOD_map_is_in_fov(player_fov->fov_map, other_position->x, other_position->y) &&
                        //                             other_alignment->type != player_alignment->type)
                        //                         {
                        //                             target_found = true;

                        //                             player_targeting->x = other_position->x;
                        //                             player_targeting->y = other_position->y;

                        //                             break;
                        //                         }
                        //                     }
                        //                 }
                        //             }
                        //         }

                        //         if (!target_found)
                        //         {
                        //             player_targeting->x = player_position->x;
                        //             player_targeting->y = player_position->y;
                        //         }
                        //     }
                        // }
                    }
                    break;
                    case 'g':
                    {
                        // inventory_t *player_inventory = (inventory_t *)component_get(game->player, COMPONENT_INVENTORY);
                        // position_t *player_position = (position_t *)component_get(game->player, COMPONENT_POSITION);

                        // if (player_inventory && player_position)
                        // {
                        //     tile_t *tile = &game->maps[player_position->level].tiles[player_position->x][player_position->y];

                        //     bool item_found = false;

                        //     for (void **iterator = TCOD_list_begin(tile->entities); iterator != TCOD_list_end(tile->entities); iterator++)
                        //     {
                        //         entity_t *other = *iterator;

                        //         pickable_t *pickable = (pickable_t *)component_get(other, COMPONENT_PICKABLE);

                        //         if (pickable)
                        //         {
                        //             game->should_update = true;

                        //             item_found = true;

                        //             entity_pick(game->player, other);

                        //             break;
                        //         }
                        //     }

                        //     if (!item_found)
                        //     {
                        //         game_log(game->player->game, player_position, TCOD_white, "There is nothing here!");
                        //     }
                        // }
                    }
                    break;
                    case 'i':
                    {
                        game->current_panel = PANEL_INVENTORY;
                        game->panel_visible = !game->panel_visible;
                    }
                    break;
                    case 'l':
                    {
                        // position_t *player_position = (position_t *)component_get(game->player, COMPONENT_POSITION);

                        // if (player_position)
                        // {
                        //     targeting_t *player_targeting = (targeting_t *)component_get(game->player, COMPONENT_TARGETING);

                        //     if (player_targeting)
                        //     {
                        //         component_remove(game->player, COMPONENT_TARGETING);
                        //     }
                        //     else
                        //     {
                        //         player_targeting = (targeting_t *)component_add(game->player, COMPONENT_TARGETING);
                        //         player_targeting->type = TARGETING_LOOK;
                        //         player_targeting->x = player_position->x;
                        //         player_targeting->y = player_position->y;
                        //     }
                        // }
                    }
                    break;
                    case 'm':
                    {
                        game->message_log_visible = !game->message_log_visible;
                    }
                    break;
                    case 'o':
                    {
                        action = ACTION_OPEN_DOOR;

                        game_log(game, game->player->level, game->player->x, game->player->y, TCOD_white, "Choose a direction");
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
                        // caster_t *player_caster = (caster_t *)component_get(game->player, COMPONENT_CASTER);

                        // if (player_caster)
                        // {
                        //     spell_t *spell = &player_caster->spells[player_caster->current];

                        //     switch (spell->type)
                        //     {
                        //     case SPELL_HEAL_SELF:
                        //     {
                        //         game->should_update = true;

                        //         entity_cast_spell(game->player);
                        //     }
                        //     break;
                        //     case SPELL_INSTAKILL:
                        //     {
                        //         position_t *player_position = (position_t *)component_get(game->player, COMPONENT_POSITION);

                        //         if (player_position)
                        //         {
                        //             targeting_t *player_targeting = (targeting_t *)component_get(game->player, COMPONENT_TARGETING);

                        //             if (player_targeting && player_targeting->type == TARGETING_ZAP)
                        //             {
                        //                 game->should_update = true;

                        //                 entity_cast_spell(game->player);

                        //                 component_remove(game->player, COMPONENT_TARGETING);
                        //             }
                        //             else
                        //             {
                        //                 player_targeting = (targeting_t *)component_add(game->player, COMPONENT_TARGETING);
                        //                 player_targeting->type = TARGETING_ZAP;

                        //                 bool target_found = false;

                        //                 {
                        //                     alignment_t *player_alignment = (alignment_t *)component_get(game->player, COMPONENT_ALIGNMENT);
                        //                     fov_t *player_fov = (fov_t *)component_get(game->player, COMPONENT_FOV);

                        //                     if (player_alignment && player_fov)
                        //                     {
                        //                         for (void **iterator = TCOD_list_begin(game->maps[player_position->level].entities); iterator != TCOD_list_end(game->maps[player_position->level].entities); iterator++)
                        //                         {
                        //                             entity_t *other = *iterator;

                        //                             alignment_t *other_alignment = (alignment_t *)component_get(other, COMPONENT_ALIGNMENT);
                        //                             position_t *other_position = (position_t *)component_get(other, COMPONENT_POSITION);

                        //                             if (other_alignment && other_position)
                        //                             {
                        //                                 if (TCOD_map_is_in_fov(player_fov->fov_map, other_position->x, other_position->y) &&
                        //                                     other_alignment->type != player_alignment->type)
                        //                                 {
                        //                                     target_found = true;

                        //                                     player_targeting->x = other_position->x;
                        //                                     player_targeting->y = other_position->y;

                        //                                     break;
                        //                                 }
                        //                             }
                        //                         }
                        //                     }
                        //                 }

                        //                 if (!target_found)
                        //                 {
                        //                     player_targeting->x = player_position->x;
                        //                     player_targeting->y = player_position->y;
                        //                 }
                        //             }
                        //         }
                        //     }
                        //     break;
                        //     }
                        // }
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
    if (game->should_update)
    {
        game->should_update = false;

        struct map *map = &game->maps[game->player->level];

        for (void **iterator = TCOD_list_begin(map->actors); iterator != TCOD_list_end(map->actors); iterator++)
        {
            struct actor *actor = *iterator;

            actor_calc_light(actor);
            actor_calc_fov(actor);
            actor_ai(actor);
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

    if (view_width < MAP_WIDTH && view_height < MAP_HEIGHT)
    {
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
    }

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

                    if (TCOD_map_is_in_fov(game->player->fov, x, y))
                    {
                        tile->seen = true;
                    }

                    for (void **iterator = TCOD_list_begin(map->objects); iterator != TCOD_list_end(map->objects); iterator++)
                    {
                        struct object *object = *iterator;

                        if (object->light_fov && TCOD_map_is_in_fov(object->light_fov, x, y))
                        {
                            tile->seen = true;
                        }
                    }

                    for (void **iterator = TCOD_list_begin(map->actors); iterator != TCOD_list_end(map->actors); iterator++)
                    {
                        struct actor *actor = *iterator;

                        if (actor->glow_fov && TCOD_map_is_in_fov(actor->glow_fov, x, y))
                        {
                            tile->seen = true;
                        }

                        if (actor->torch_fov && TCOD_map_is_in_fov(actor->torch_fov, x, y))
                        {
                            tile->seen = true;
                        }
                    }

                    if (!tile->seen)
                    {
                        continue;
                    }

                    TCOD_color_t color = game->tile_common.shadow_color;

                    if (TCOD_map_is_in_fov(game->player->fov, x, y))
                    {
                        for (void **iterator = TCOD_list_begin(map->objects); iterator != TCOD_list_end(map->objects); iterator++)
                        {
                            struct object *object = *iterator;

                            if (object->light_fov && TCOD_map_is_in_fov(object->light_fov, x, y))
                            {
                                float r2 = powf((float)game->object_info[object->type].light_radius, 2);
                                float d = powf((float)(x - object->x), 2) + powf((float)(y - object->y), 2);
                                float l = CLAMP(0.0f, 1.0f, (r2 - d) / r2);

                                color = TCOD_color_lerp(color, TCOD_color_lerp(tile_info->color, game->object_info[object->type].light_color, l), l);
                            }
                        }

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

                            if (actor->torch_fov && TCOD_map_is_in_fov(actor->torch_fov, x, y))
                            {
                                float r2 = powf((float)game->actor_common.torch_radius, 2);
                                float d = powf(x - actor->x + dx, 2) + powf(y - actor->y + dy, 2);
                                float l = CLAMP(0.0f, 1.0f, (r2 - d) / r2 + di);

                                color = TCOD_color_lerp(color, TCOD_color_lerp(tile_info->color, game->actor_common.torch_color, l), l);
                            }
                        }
                    }

                    TCOD_console_set_char_foreground(NULL, x - view_x, y - view_y, color);
                    TCOD_console_set_char(NULL, x - view_x, y - view_y, tile_info->glyph);
                }
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

        for (void **iterator = TCOD_list_begin(map->actors); iterator != TCOD_list_end(map->actors); iterator++)
        {
            struct actor *actor = *iterator;

            if (TCOD_map_is_in_fov(game->player->fov, actor->x, actor->y))
            {
                TCOD_color_t color = game->class_info[actor->class].color;

                // flash_t *flash = (flash_t *)component_get(entity, COMPONENT_FLASH);

                // if (flash)
                // {
                //     color = TCOD_color_lerp(appearance->color, flash->color, flash->fade);
                // }

                TCOD_console_set_char_foreground(NULL, actor->x - view_x, actor->y - view_y, color);
                TCOD_console_set_char(NULL, actor->x - view_x, actor->y - view_y, game->race_info[actor->race].glyph);
            }
        }
    }

    // {
    //     targeting_t *player_targeting = (targeting_t *)component_get(game->player, COMPONENT_TARGETING);

    //     if (player_targeting)
    //     {
    //         TCOD_console_set_char_foreground(NULL, player_targeting->x - view_x, player_targeting->y - view_y, TCOD_red);
    //         TCOD_console_set_char(NULL, player_targeting->x - view_x, player_targeting->y - view_y, 'X');

    //         switch (player_targeting->type)
    //         {
    //         case TARGETING_LOOK:
    //         {
    //             tile_t *tile = &game->maps[player_position->level].tiles[player_targeting->x][player_targeting->y];

    //             if (TCOD_map_is_in_fov(player_fov->fov_map, player_targeting->x, player_targeting->y))
    //             {
    //                 entity_t *entity = TCOD_list_peek(tile->entities);
    //                 appearance_t *appearance = (appearance_t *)component_get(entity, COMPONENT_APPEARANCE);

    //                 if (entity == NULL && appearance == NULL)
    //                 {
    //                     TCOD_console_print_ex(NULL, console_width / 2, message_log_y - 2, TCOD_BKGND_NONE, TCOD_CENTER, game->tile_info[tile->type].name);
    //                 }
    //                 else
    //                 {
    //                     TCOD_console_print_ex(NULL, console_width / 2, message_log_y - 2, TCOD_BKGND_NONE, TCOD_CENTER, appearance->name);
    //                 }
    //             }
    //             else
    //             {
    //                 if (tile->seen)
    //                 {
    //                     TCOD_console_print_ex(NULL, console_width / 2, message_log_y - 2, TCOD_BKGND_NONE, TCOD_CENTER, "%s (remembered)", game->tile_info[tile->type].name);
    //                 }
    //                 else
    //                 {
    //                     TCOD_console_print_ex(NULL, console_width / 2, message_log_y - 2, TCOD_BKGND_NONE, TCOD_CENTER, "Unknown");
    //                 }
    //             }
    //             break;
    //         }
    //         }
    //     }
    // }

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
            TCOD_console_print(message_log, message_log_x + 1, y, message->text);

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

        switch (game->current_panel)
        {
        case PANEL_CHARACTER:
        {
            TCOD_console_set_default_foreground(panel, TCOD_white);
            TCOD_console_print_frame(panel, 0, 0, panel_width, panel_height, false, TCOD_BKGND_SET, "Character");
        }
        break;
        case PANEL_INVENTORY:
        {
            TCOD_console_set_default_foreground(panel, TCOD_white);
            TCOD_console_print_frame(panel, 0, 0, panel_width, panel_height, false, TCOD_BKGND_SET, "Inventory");
        }
        break;
        }

        TCOD_console_blit(panel, 0, 0, panel_width, panel_height, NULL, panel_x, panel_y, 1, 1);
    }

    TCOD_console_print(NULL, 0, 0, "Turn: %d", game->turn);

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
