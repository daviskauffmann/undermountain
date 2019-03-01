#include <platform/platform.h>

static void init(void);
static bool handleEvent(TCOD_event_t ev, TCOD_key_t key, TCOD_mouse_t mouse);
static void update(void);
static void render(void);
static void quit(void);

struct state about_state = {
    &init,
    &handleEvent,
    &update,
    &render,
    &quit
};

static void init(void)
{
}

static bool handleEvent(TCOD_event_t ev, TCOD_key_t key, TCOD_mouse_t mouse)
{
    switch (ev)
    {
    case TCOD_EVENT_KEY_PRESS:
    {
        switch (key.vk)
        {
        case TCODK_ESCAPE:
        {
            state_set(&menu_state);
        }
        break;
        }
    }
    break;
    case TCOD_EVENT_MOUSE_PRESS:
    {
        if (mouse.rbutton)
        {
            state_set(&menu_state);
        }
    }
    break;
    }

    return false;
}

static void update(void)
{
}

static void render(void)
{
    int y = 1;

    y += TCOD_console_printf_rect(NULL, 1, y, console_width - 2, console_height - 2, "Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua. Ut enim ad minim veniam, quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea commodo consequat. Duis aute irure dolor in reprehenderit in voluptate velit esse cillum dolore eu fugiat nulla pariatur. Excepteur sint occaecat cupidatat non proident, sunt in culpa qui officia deserunt mollit anim id est laborum.");

    TCOD_console_printf_rect(NULL, 1, y + 1, console_width - 2, console_height - 2, "Press ESC or R-Mouse to return.");

    TCOD_console_set_default_foreground(NULL, TCOD_white);
    TCOD_console_printf_frame(NULL, 0, 0, console_width, console_height, false, TCOD_BKGND_SET, TITLE);
}

static void quit(void)
{
}
