#include <libtcod.h>

#include "view.h"
#include "config.h"
#include "game.h"
#include "world.h"

void view_update(void)
{
    view_right = screen_width;
    view_bottom = screen_height;
    view_left = player->x - view_right / 2;
    view_top = player->y - view_bottom / 2;

    if (CONSTRAIN_VIEW)
    {
        view_left = view_left < 0
                        ? 0
                        : view_left + view_right > MAP_WIDTH
                              ? MAP_WIDTH - view_right
                              : view_left;
        view_top = view_top < 0
                       ? 0
                       : view_top + view_bottom > MAP_HEIGHT
                             ? MAP_HEIGHT - view_bottom
                             : view_top;
    }
}