#include "menu_scene.h"

#include "../../config.h"
#include "../../game/color.h"
#include "../../game/world.h"
#include "../../print.h"
#include "../../scene.h"
#include "../../util.h"
#include "../create/create_scene.h"
#include "../game/game_scene.h"
#include "option.h"
#include <libtcod.h>

static struct option_data option_data[NUM_OPTIONS];

static enum option selected_option;

static void init(const struct scene *const previous_scene)
{
    previous_scene;

    option_data[OPTION_START] = (struct option_data){
        .text = "Start",
    };
    option_data[OPTION_QUIT] = (struct option_data){
        .text = "Quit",
    };
}

static void uninit(void)
{
}

static struct scene *handle_event(const SDL_Event *const event)
{
    switch (event->type)
    {
    case SDL_KEYDOWN:
    {
        switch (event->key.keysym.sym)
        {
        case SDLK_RETURN:
        {
            switch (selected_option)
            {
            case OPTION_START:
            {
                if (file_exists(SAVE_PATH))
                {
                    // TODO: prompt whether the player wants to overwrite the save with a new character
                    // if so, go to character creation
                    world_init();
                    world_load(SAVE_PATH);

                    menu_scene.uninit();
                    game_scene.init(&menu_scene);
                    return &game_scene;
                }
                else
                {
                    menu_scene.uninit();
                    create_scene.init(&menu_scene);
                    return &create_scene;
                }
            }
            break;
            case OPTION_QUIT:
            {
                menu_scene.uninit();
                return NULL;
            }
            break;
            }
        }
        break;
        case SDLK_ESCAPE:
        {
            menu_scene.uninit();
            return NULL;
        }
        break;
        case SDLK_KP_2:
        case SDLK_DOWN:
        {
            if (selected_option == NUM_OPTIONS - 1)
            {
                selected_option = 0;
            }
            else
            {
                selected_option++;
            }
        }
        break;
        case SDLK_KP_8:
        case SDLK_UP:
        {
            if (selected_option == 0)
            {
                selected_option = NUM_OPTIONS - 1;
            }
            else
            {
                selected_option--;
            }
        }
        break;
        }
    }
    break;
    }

    return &menu_scene;
}

static struct scene *update(TCOD_Console *const console, const float delta_time)
{
    delta_time;

    int y = console_height / 2;

    for (enum option option = OPTION_START; option < NUM_OPTIONS; option++)
    {
        console_print(
            console,
            console_width / 2,
            y++,
            option == selected_option ? &color_yellow : &color_white,
            &color_black,
            TCOD_BKGND_NONE,
            TCOD_CENTER,
            "%s",
            option_data[option].text);
    }

    TCOD_console_printf_frame(
        console,
        0, 0, console_width, console_height,
        false,
        TCOD_BKGND_NONE,
        TITLE);

    return &menu_scene;
}

struct scene menu_scene =
    {&init,
     &uninit,
     &handle_event,
     &update};
