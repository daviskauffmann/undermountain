#include <platform/platform.h>

int platform_run(void)
{
    config_load();

    TCOD_sys_set_fps(FPS);
    TCOD_console_set_custom_font(font_file, font_flags, font_char_horiz, font_char_vertic);

    TCOD_console_init_root(console_width, console_height, TITLE, fullscreen, console_renderer);

    input_init();
    ui_init();
    renderer_init();

    assets_load();
    game_init();

    while (!TCOD_console_is_window_closed() && !input->should_quit)
    {
        float delta_time = TCOD_sys_get_last_frame_length();

        input_handle(delta_time);
        game_update(delta_time);
        ui_update(delta_time);
        renderer_draw(delta_time);
    }

    game_quit();

    renderer_quit();
    ui_quit();
    input_quit();

    TCOD_console_delete(NULL);

    config_save();

    return 0;
}
