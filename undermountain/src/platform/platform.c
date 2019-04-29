#include <platform/platform.h>

int platform_run(void)
{
    config_load();

    TCOD_sys_set_fps(FPS);
    TCOD_console_set_custom_font(font_file, font_flags, font_char_horiz, font_char_vertic);
    TCOD_console_init_root(console_width, console_height, TITLE, fullscreen, console_renderer);

    struct state *state = &menu_state;
    state->init(NULL);

    while (!TCOD_console_is_window_closed())
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
            case TCODK_ENTER:
            {
                if (key.lalt)
                {
                    fullscreen = !fullscreen;

                    TCOD_console_set_fullscreen(fullscreen);
                }
            }
            break;
            }
        }
        break;
        }

        if (!(state = state->handleEvent(ev, key, mouse)))
        {
            break;
        }

        if (!(state = state->update(TCOD_sys_get_last_frame_length())))
        {
            break;
        }

        TCOD_console_set_default_background(NULL, TCOD_black);
        TCOD_console_set_default_foreground(NULL, TCOD_white);
        TCOD_console_clear(NULL);
        state->render(NULL);
        TCOD_console_flush();
    }

    if (game)
    {
        if (game->state != GAME_STATE_LOSE)
        {
            game_save(SAVE_PATH);
        }

        game_quit();
    }

    TCOD_console_delete(NULL);
    TCOD_quit();

    config_save();

    return 0;
}
