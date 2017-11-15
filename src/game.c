#include <libtcod.h>

#include "CMemLeak.h"
#include "game.h"
#include "system.h"

void game_init(void)
{
    world_init();
    gfx_init();
}

void game_new()
{
    turn = 0;

    map_t *map = map_create(0);

    TCOD_list_push(maps, map);

    player = actor_create(ACTOR_TYPE_WARRIOR, map, map->stair_up_x, map->stair_up_y, NULL, "Blinky");

    TCOD_list_push(map->actors, player);
    map->tiles[player->x][player->y].actor = player;

    for (int i = 0; i < 30; i++)
    {
        item_t *item = item_create(TCOD_random_get_int(NULL, 0, NUM_ITEM_TYPES - 1), player->x, player->y, 1);

        TCOD_list_push(map->items, item);
        TCOD_list_push(player->items, item);
    }

    TCOD_list_push(player->spells, &spell_info[SPELL_INSTAKILL]);

    msg_log(player->map, player->x, player->y, TCOD_white, "Hail, %s!", actor_get_name(player));

    actor_t *pet = actor_create(ACTOR_TYPE_DOG, map, player->x + 1, player->y, &ai_pet, "Spot");

    TCOD_list_push(map->actors, pet);
    map->tiles[pet->x][pet->y].actor = pet;
}

void game_input(void)
{
    TCOD_key_t key;
    TCOD_mouse_t mouse;
    TCOD_event_t ev = TCOD_sys_check_for_event(TCOD_EVENT_ANY, &key, &mouse);

    switch (ev)
    {
    case TCOD_EVENT_KEY_PRESS:
    {
        tile_t *tile = &player->map->tiles[player->x][player->y];

        switch (key.vk)
        {
        case TCODK_ESCAPE:
        {
            game_status = GAME_STATUS_QUIT;

            break;
        }
        case TCODK_PAGEDOWN:
        {
            panel_content_scroll_down();

            break;
        }
        case TCODK_PAGEUP:
        {
            panel_content_scroll_up();

            break;
        }
        case TCODK_KP1:
        {
            game_status = GAME_STATUS_UPDATE;

            int x = player->x - 1;
            int y = player->y + 1;

            if (key.lctrl)
            {
                actor_swing(player, x, y);
            }
            else
            {
                actor_default_action(player, x, y);
            }

            break;
        }
        case TCODK_KP2:
        case TCODK_DOWN:
        {
            if (panel_visible)
            {
                panel_content_idx_up();
            }
            else
            {
                game_status = GAME_STATUS_UPDATE;

                int x = player->x;
                int y = player->y + 1;

                if (key.lctrl)
                {
                    actor_swing(player, x, y);
                }
                else
                {
                    actor_default_action(player, x, y);
                }
            }

            break;
        }
        case TCODK_KP3:
        {
            game_status = GAME_STATUS_UPDATE;

            int x = player->x + 1;
            int y = player->y + 1;

            if (key.lctrl)
            {
                actor_swing(player, x, y);
            }
            else
            {
                actor_default_action(player, x, y);
            }

            break;
        }
        case TCODK_KP4:
        {
            game_status = GAME_STATUS_UPDATE;

            int x = player->x - 1;
            int y = player->y;

            if (key.lctrl)
            {
                actor_swing(player, x, y);
            }
            else
            {
                actor_default_action(player, x, y);
            }

            break;
        }
        case TCODK_KP5:
        {
            game_status = GAME_STATUS_UPDATE;

            break;
        }
        case TCODK_KP6:
        {
            game_status = GAME_STATUS_UPDATE;

            int x = player->x + 1;
            int y = player->y;

            if (key.lctrl)
            {
                actor_swing(player, x, y);
            }
            else
            {
                actor_default_action(player, x, y);
            }

            break;
        }
        case TCODK_KP7:
        {
            game_status = GAME_STATUS_UPDATE;

            int x = player->x - 1;
            int y = player->y - 1;

            if (key.lctrl)
            {
                actor_swing(player, x, y);
            }
            else
            {
                actor_default_action(player, x, y);
            }

            break;
        }
        case TCODK_KP8:
        case TCODK_UP:
        {
            if (panel_visible)
            {
                panel_content_idx_down();
            }
            else
            {
                game_status = GAME_STATUS_UPDATE;

                int x = player->x;
                int y = player->y - 1;

                if (key.lctrl)
                {
                    actor_swing(player, x, y);
                }
                else
                {
                    actor_default_action(player, x, y);
                }
            }

            break;
        }
        case TCODK_KP9:
        {
            game_status = GAME_STATUS_UPDATE;

            int x = player->x + 1;
            int y = player->y - 1;

            if (key.lctrl)
            {
                actor_swing(player, x, y);
            }
            else
            {
                actor_default_action(player, x, y);
            }

            break;
        }
        case TCODK_CHAR:
        {
            switch (key.c)
            {
            case 'b':
            {
                panel_toggle(CONTENT_SPELLBOOK);

                break;
            }
            case 'c':
            {
                panel_toggle(CONTENT_CHARACTER);

                break;
            }
            case 'i':
            {
                panel_toggle(CONTENT_INVENTORY);

                break;
            }
            case 'g':
            {
                game_status = GAME_STATUS_UPDATE;

                if (TCOD_list_peek(tile->items) != NULL)
                {
                    actor_item_take(player, tile->items);
                }

                break;
            }
            case 'l':
            {
                if (key.lctrl)
                {
                    game_uninit();
                    game_init();
                    game_load();
                }
                else
                {
                    msg_visible = !msg_visible;
                }

                break;
            }
            case 'r':
            {
                game_uninit();
                game_init();
                game_new();

                break;
            }
            case 's':
            {
                if (key.lctrl)
                {
                    game_save();
                }

                break;
            }
            case 't':
            {
                game_status = GAME_STATUS_UPDATE;

                if (player->light != ACTOR_LIGHT_TYPE_TORCH)
                {
                    player->light = ACTOR_LIGHT_TYPE_TORCH;
                }
                else
                {
                    player->light = ACTOR_LIGHT_TYPE_DEFAULT;
                }

                break;
            }
            case 'y':
            {
                game_status = GAME_STATUS_UPDATE;

                light_t *light = tile->light;

                if (light != NULL)
                {
                    actor_light_toggle(player, light);
                }

                break;
            }
            case ',':
            {
                game_status = GAME_STATUS_UPDATE;

                actor_ascend(player);

                break;
            }
            case '.':
            {
                game_status = GAME_STATUS_UPDATE;

                actor_descend(player);

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

void game_update(void)
{
    if (game_status == GAME_STATUS_UPDATE)
    {
        world_update();

        turn++;

        game_status = GAME_STATUS_WAITING;
    }
}

void game_save(void)
{
}

void game_load(void)
{
}

void game_draw(void)
{
    gfx_draw();
}

void game_uninit(void)
{
    world_uninit();
    gfx_uninit();
}