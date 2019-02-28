#include <platform/platform.h>

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
            platform_set_state(&menu_state);
        }
        break;
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
}

static void quit(void)
{
    if (game->state != GAME_STATE_LOSE)
    {
        game_save();
    }

    game_quit();
}

struct state game_state = {
    &init,
    &handleEvent,
    &update,
    &render,
    &quit
};
