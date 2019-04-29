#include <platform/platform.h>

static void init(struct state *previous_state);
static struct state *handleEvent(TCOD_event_t ev, TCOD_key_t key, TCOD_mouse_t mouse);
static struct state *update(float delta);
static void render(TCOD_console_t console);
static void quit(void);

struct state about_state = {
    &init,
    &handleEvent,
    &update,
    &render,
    &quit
};

static void init(struct state *previous_state)
{
}

static struct state *handleEvent(TCOD_event_t ev, TCOD_key_t key, TCOD_mouse_t mouse)
{
    switch (ev)
    {
    case TCOD_EVENT_KEY_PRESS:
    {
        switch (key.vk)
        {
        case TCODK_ESCAPE:
        {
            about_state.quit();
            menu_state.init(&about_state);
            return &menu_state;
        }
        break;
        }
    }
    break;
    case TCOD_EVENT_MOUSE_PRESS:
    {
        if (mouse.rbutton)
        {
            about_state.quit();
            menu_state.init(&about_state);
            return &menu_state;
        }
    }
    break;
    }

    return &about_state;
}

static struct state *update(float delta)
{
    return &about_state;
}

static void render(TCOD_console_t console)
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
