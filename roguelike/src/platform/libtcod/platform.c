#ifdef PLATFORM_LIBTCOD

#include <platform/libtcod/libtcod.h>

int platform_run(void)
{
    config_load();

    TCOD_sys_set_fps(FPS);
    TCOD_console_set_custom_font(font_file, font_flags, font_char_horiz, font_char_vertic);

    TCOD_console_init_root(console_width, console_height, WINDOW_TITLE, fullscreen, console_renderer);

    input_init();
    renderer_init();

    assets_load();
    game_init();
    ui_init();

    while (!TCOD_console_is_window_closed() && !game->should_quit)
    {
        input_handle();

        game_update();
        ui_update();

        renderer_draw();
    }

    ui_quit();
    game_quit();

    renderer_quit();
    input_quit();

    TCOD_console_delete(NULL);

    config_save();

    return 0;
}

#endif
