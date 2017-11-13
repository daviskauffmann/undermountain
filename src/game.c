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

    player = actor_create(ACTOR_TYPE_PLAYER, map, map->stair_up_x, map->stair_up_y);

    TCOD_list_push(map->actors, player);
    map->tiles[player->x][player->y].actor = player;

    for (int i = 0; i < 30; i++)
    {
        item_t *item = item_create_random(player->x, player->y);

        TCOD_list_push(map->items, item);
        TCOD_list_push(player->items, item);
    }

    TCOD_list_push(player->spells, &spell[SPELL_INSTAKILL]);

    msg_log(player->map, player->x, player->y, TCOD_white, "Hail, %s!\nWelcome to the dungeon!", actor_name[player->type]);

    actor_t *pet = actor_create(ACTOR_TYPE_PET, map, player->x + 1, player->y);

    TCOD_list_push(map->actors, pet);
    map->tiles[pet->x][pet->y].actor = pet;
}

void game_update(void)
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
            game_status = GAME_STATUS_QUIT;

            break;
        }
        case TCODK_KP1:
        {
            game_status = GAME_STATUS_UPDATE;

            actor_move(player, player->x - 1, player->y + 1);

            break;
        }
        case TCODK_KP2:
        {
            game_status = GAME_STATUS_UPDATE;

            actor_move(player, player->x, player->y + 1);

            break;
        }
        case TCODK_KP3:
        {
            game_status = GAME_STATUS_UPDATE;

            actor_move(player, player->x + 1, player->y + 1);

            break;
        }
        case TCODK_KP4:
        {
            game_status = GAME_STATUS_UPDATE;

            actor_move(player, player->x - 1, player->y);

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

            actor_move(player, player->x + 1, player->y);

            break;
        }
        case TCODK_KP7:
        {
            game_status = GAME_STATUS_UPDATE;

            actor_move(player, player->x - 1, player->y - 1);

            break;
        }
        case TCODK_KP8:
        {
            game_status = GAME_STATUS_UPDATE;

            actor_move(player, player->x, player->y - 1);

            break;
        }
        case TCODK_KP9:
        {
            game_status = GAME_STATUS_UPDATE;

            actor_move(player, player->x + 1, player->y - 1);

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
                if (player->light != ACTOR_LIGHT_TORCH)
                {
                    player->light = ACTOR_LIGHT_TORCH;
                }
                else
                {
                    player->light = ACTOR_LIGHT_DEFAULT;
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