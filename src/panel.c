#include <libtcod.h>
#include <string.h>
#include <stdio.h>

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
    content_scroll[CONTENT_SPELLBOOK] = 0;

    content_selected_idx[CONTENT_CHARACTER] = 0;
    content_selected_idx[CONTENT_INVENTORY] = 0;
    content_selected_idx[CONTENT_SPELLBOOK] = 0;
}

void panel_toggle(content_t new_content)
{
    if (panel_visible && content == new_content)
    {
        panel_visible = false;
    }
    else
    {
        content = new_content;

        panel_visible = true;
    }
}

void panel_content_idx_up(void)
{
    content_selected_idx[content]++;
}

void panel_content_idx_down(void)
{
    content_selected_idx[content]--;
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

void panel_draw(void)
{
    if (panel_visible)
    {
        TCOD_console_set_default_background(panel, background_color);
        TCOD_console_set_default_foreground(panel, foreground_color);
        TCOD_console_clear(panel);

        switch (content)
        {
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
            int i = 0;
            int y = 1 - content_scroll[content];
            for (void **iterator = TCOD_list_begin(player->items); iterator != TCOD_list_end(player->items); iterator++)
            {
                item_t *item = *iterator;

                TCOD_color_t color = i == content_selected_idx[content] ? TCOD_yellow : item_info[item->type].color;

                TCOD_console_set_default_foreground(panel, color);
                TCOD_console_print(panel, 1, y, item_info[item->type].name);

                i++;
                y++;
            }

            TCOD_console_set_default_foreground(panel, foreground_color);
            TCOD_console_print_frame(panel, 0, 0, panel_width, panel_height, false, TCOD_BKGND_SET, "Inventory");

            break;
        }
        case CONTENT_SPELLBOOK:
        {
            TCOD_console_print_frame(panel, 0, 0, panel_width, panel_height, false, TCOD_BKGND_SET, "Spellbook");

            break;
        }
        }

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