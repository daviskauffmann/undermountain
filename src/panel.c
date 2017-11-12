#include <libtcod.h>
#include <string.h>

#include "CMemLeak.h"
#include "system.h"
#include "game.h"

static TCOD_console_t panel;

void panel_init(void)
{
    panel = TCOD_console_new(screen_width, screen_height);
    panel_visible = false;

    content_scroll[CONTENT_CHARACTER] = 0;
    content_scroll[CONTENT_INVENTORY] = 0;
}

void panel_toggle(content_t new_content, content_data_t data)
{
    if (panel_visible && content == new_content)
    {
        panel_visible = false;
    }
    else
    {
        content = new_content;
        content_data = data;

        panel_visible = true;
    }
}

void panel_content_scroll_down(void)
{
    if (content_scroll[content] + panel_height < content_height[content])
    {
        content_scroll[content]++;
    }
}

void panel_content_scroll_up(void)
{
    if (content_scroll[content] > 0)
    {
        content_scroll[content]--;
    }
}

bool panel_is_inside(int x, int y)
{
    return x >= panel_x && x < panel_x + panel_width && y >= panel_y && y < panel_y + panel_height;
}

void panel_draw_turn(void)
{
    if (panel_visible)
    {
        TCOD_console_set_default_background(panel, background_color);
        TCOD_console_set_default_foreground(panel, foreground_color);
        TCOD_console_clear(panel);

        switch (content)
        {
        case CONTENT_DEBUG:
        {
            TCOD_console_print_frame(panel, 0, 0, panel_width, panel_height, false, TCOD_BKGND_SET, "Debug");

            break;
        }
        case CONTENT_CHARACTER:
        {
            TCOD_console_print_ex(panel, 1, 1 - content_scroll[content], TCOD_BKGND_NONE, TCOD_LEFT, "HP: 15 / 20");
            TCOD_console_print_ex(panel, 1, 2 - content_scroll[content], TCOD_BKGND_NONE, TCOD_LEFT, "MP:  7 / 16");

            TCOD_console_print_ex(panel, 1, 4 - content_scroll[content], TCOD_BKGND_NONE, TCOD_LEFT, "STR: 16");
            TCOD_console_print_ex(panel, 1, 5 - content_scroll[content], TCOD_BKGND_NONE, TCOD_LEFT, "DEX: 14");
            TCOD_console_print_ex(panel, 1, 6 - content_scroll[content], TCOD_BKGND_NONE, TCOD_LEFT, "CON: 12");
            TCOD_console_print_ex(panel, 1, 7 - content_scroll[content], TCOD_BKGND_NONE, TCOD_LEFT, "INT: 10");
            TCOD_console_print_ex(panel, 1, 8 - content_scroll[content], TCOD_BKGND_NONE, TCOD_LEFT, "WIS: 8");
            TCOD_console_print_ex(panel, 1, 9 - content_scroll[content], TCOD_BKGND_NONE, TCOD_LEFT, "CHA: 10");

            TCOD_console_print_ex(panel, 1, 11 - content_scroll[content], TCOD_BKGND_NONE, TCOD_LEFT, "R-Hand: Sword");
            TCOD_console_print_ex(panel, 1, 12 - content_scroll[content], TCOD_BKGND_NONE, TCOD_LEFT, "L-Hand: Shield");
            TCOD_console_print_ex(panel, 1, 13 - content_scroll[content], TCOD_BKGND_NONE, TCOD_LEFT, "Head  : Helm");
            TCOD_console_print_ex(panel, 1, 14 - content_scroll[content], TCOD_BKGND_NONE, TCOD_LEFT, "Chest : Cuirass");
            TCOD_console_print_ex(panel, 1, 15 - content_scroll[content], TCOD_BKGND_NONE, TCOD_LEFT, "Legs  : Greaves");
            TCOD_console_print_ex(panel, 1, 16 - content_scroll[content], TCOD_BKGND_NONE, TCOD_LEFT, "Feet  : Boots");

            TCOD_console_print_frame(panel, 0, 0, panel_width, panel_height, false, TCOD_BKGND_SET, "Character");

            break;
        }
        case CONTENT_INVENTORY:
        {
            TCOD_console_set_default_foreground(panel, foreground_color);
            TCOD_console_print_frame(panel, 0, 0, panel_width, panel_height, false, TCOD_BKGND_SET, "Inventory");

            break;
        }
        case CONTENT_EXAMINE_TILE:
        {
            tile_t *tile = content_data.tile;

            TCOD_console_print_frame(panel, 0, 0, panel_width, panel_height, false, TCOD_BKGND_SET, "Examine Tile");

            break;
        }
        case CONTENT_EXAMINE_ACTOR:
        {
            actor_t *actor = content_data.actor;

            TCOD_console_print_ex(panel, 1, 1 - content_scroll[content], TCOD_BKGND_NONE, TCOD_LEFT, "Energy: %d", actor->energy);

            TCOD_console_print_frame(panel, 0, 0, panel_width, panel_height, false, TCOD_BKGND_SET, "Examine Actor");

            break;
        }
        case CONTENT_EXAMINE_ITEM:
        {
            item_t *item = content_data.item;

            TCOD_console_print_frame(panel, 0, 0, panel_width, panel_height, false, TCOD_BKGND_SET, "Examine Item");

            break;
        }
        }
    }
}

void panel_draw_tick(void)
{
    if (panel_visible)
    {
        switch (content)
        {
        case CONTENT_DEBUG:
        {
            TCOD_console_set_default_foreground(panel, foreground_color);
            TCOD_console_print_ex(panel, 1, 1, TCOD_BKGND_SET, TCOD_LEFT, "Turn: %d", turn);
            TCOD_console_print_ex(panel, 1, 2, TCOD_BKGND_SET, TCOD_LEFT, "Lvl: %d", player->map->level);
            TCOD_console_print_ex(panel, 1, 3, TCOD_BKGND_SET, TCOD_LEFT, "Loc: (%d, %d)", player->x, player->y);
            TCOD_console_print_ex(panel, 1, 4, TCOD_BKGND_SET, TCOD_LEFT, "Tile: (%d, %d)", mouse_tile_x, mouse_tile_y);
            TCOD_console_print_ex(panel, 1, 5, TCOD_BKGND_SET, TCOD_LEFT, "Mouse: (%d, %d)", mouse_x, mouse_y);

            break;
        }
        case CONTENT_INVENTORY:
        {
            int y = 1;
            for (void **i = TCOD_list_begin(player->items); i != TCOD_list_end(player->items); i++)
            {
                int item_y = panel_y + y - content_scroll[content];

                if (item_y > 0 && item_y < panel_height - 1)
                {
                    item_t *item = *i;

                    TCOD_color_t color = item->color;

                    if (mouse_x > panel_x && mouse_x < panel_x + strlen("{name}") + 1 && mouse_y == item_y)
                    {
                        color = TCOD_yellow;
                    }

                    TCOD_console_set_default_foreground(panel, color);
                    TCOD_console_print_ex(panel, 1, item_y, TCOD_BKGND_NONE, TCOD_LEFT, "{name}");
                }

                y++;
            }

            break;
        }
        }

        TCOD_console_set_default_foreground(panel, foreground_color);

        if (content_scroll[content] > 0)
        {
            TCOD_console_print(panel, panel_width - 2, panel_y + 1, "^");
        }

        if (content_scroll[content] + panel_height < content_height[content])
        {
            TCOD_console_print(panel, panel_width - 2, panel_height - 2, "v");
        }

        TCOD_console_blit(panel, 0, 0, panel_width, panel_height, NULL, panel_x, panel_y, 1, 1);
    }
}

void panel_uninit(void)
{
    TCOD_console_delete(panel);
}