#include <libtcod.h>

#include "CMemLeak.h"
#include "menu.h"

menu_input_t menu_input(void)
{
    TCOD_key_t key;
    TCOD_mouse_t mouse;
    TCOD_event_t ev = TCOD_sys_check_for_event(TCOD_EVENT_ANY, &key, &mouse);

    switch (key.vk)
    {
    case TCODK_SPACE:
        return MENU_INPUT_START;

    case TCODK_TAB:
        return MENU_INPUT_QUIT;
    }

    return MENU_INPUT_NONE;
}

void menu_draw(void)
{
    TCOD_console_clear(NULL);
    TCOD_console_flush();
}