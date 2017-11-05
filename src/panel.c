#include <libtcod.h>

#include "CMemLeak.h"
#include "system.h"
#include "game.h"

void panel_draw(void)
{
    switch (content)
    {
    case CONTENT_CHARACTER:
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

    case CONTENT_INVENTORY:
        int y = 1;
        for (void **i = TCOD_list_begin(player->items); i != TCOD_list_end(player->items); i++)
        {
            item_t *item = *i;

            TCOD_console_set_default_foreground(panel, item->color);
            TCOD_console_print_ex(panel, 1, y - content_scroll[content], TCOD_BKGND_NONE, TCOD_LEFT, "{name}");

            y++;
        }

        TCOD_console_set_default_foreground(panel, foreground_color);
        TCOD_console_print_frame(panel, 0, 0, panel_width, panel_height, false, TCOD_BKGND_SET, "Inventory");

        break;
    }

    TCOD_console_blit(panel, 0, 0, panel_width, panel_height, NULL, panel_x, panel_y, 1, 1);
}