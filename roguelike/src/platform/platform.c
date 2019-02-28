#include <platform/platform.h>

static struct state *current_state = &menu_state;

int platform_run(void)
{
    config_load();

    TCOD_sys_set_fps(FPS);
    TCOD_console_set_custom_font(font_file, font_flags, font_char_horiz, font_char_vertic);
    TCOD_console_init_root(console_width, console_height, TITLE, fullscreen, console_renderer);

    // TODO: move this?
    assets_load();

    current_state->init();

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

        if (current_state->handleEvent(ev, key, mouse))
        {
            break;
        }

        current_state->update();

        TCOD_console_set_default_background(NULL, TCOD_black);
        TCOD_console_set_default_foreground(NULL, TCOD_white);
        TCOD_console_clear(NULL);
        current_state->render();
        TCOD_console_flush();

        //input_handle();

        //if (input->request_close)
        //{
        //    break;
        //}

        //if (ui->state == UI_STATE_GAME)
        //{
        //    game_update();

        //    if (input->took_turn)
        //    {
        //        input->took_turn = false;

        //        game_turn();
        //    }
        //}

        //ui_update();
        //renderer_draw();
    }

    //if (ui->state == UI_STATE_GAME)
    //{
    //    if (game->state != GAME_STATE_LOSE)
    //    {
    //        game_save();
    //    }

    //    game_quit();
    //}

    current_state->quit();

    TCOD_console_delete(NULL);
    TCOD_quit();

    config_save();

    return 0;
}

void platform_set_state(struct state *state)
{
    current_state->quit();
    current_state = state;
    current_state->init();
}

bool file_exists(const char *filename)
{
    FILE *file;
    if ((file = fopen(filename, "r")))
    {
        fclose(file);

        return true;
    }
    else
    {
        return false;
    }
}
