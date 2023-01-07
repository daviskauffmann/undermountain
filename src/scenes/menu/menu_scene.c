#include "menu_scene.h"

#include "../../config.h"
#include "../../game/world.h"
#include "../../print.h"
#include "../../scene.h"
#include "../create/create_scene.h"
#include "../game/game_scene.h"
#include "option.h"
#include <libtcod.h>

static int mouse_x;
static int mouse_y;

static struct option_datum option_data[NUM_OPTIONS];

static enum option option_mouseover(void)
{
    int y = 1;

    for (enum option option = 1; option < NUM_OPTIONS; option++)
    {
        if (mouse_y == y)
        {
            return option;
        }

        y++;
    }

    return OPTION_NONE;
}

static struct scene *select_option(enum option option)
{
    switch (option)
    {
    case OPTION_NONE:
    {
    }
    break;
    case OPTION_START:
    {
        if (TCOD_sys_file_exists(SAVE_PATH))
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
    case NUM_OPTIONS:
        break;
    }

    return &menu_scene;
}

static void init(struct scene *previous_scene)
{
    option_data[OPTION_START].text = "Start";
    option_data[OPTION_QUIT].text = "Quit";

    mouse_x = -1;
    mouse_y = -1;
}

static void uninit(void)
{
}

static struct scene *handle_event(SDL_Event *event)
{
    switch (event->type)
    {
    case SDL_KEYDOWN:
    {
        switch (event->key.keysym.sym)
        {
        case SDLK_ESCAPE:
        {
            return NULL;
        }
        break;
        case SDLK_a:
        case SDLK_b:
        case SDLK_c:
        case SDLK_d:
        case SDLK_e:
        case SDLK_f:
        case SDLK_g:
        case SDLK_h:
        case SDLK_i:
        case SDLK_j:
        case SDLK_k:
        case SDLK_l:
        case SDLK_m:
        case SDLK_n:
        case SDLK_o:
        case SDLK_p:
        case SDLK_q:
        case SDLK_r:
        case SDLK_s:
        case SDLK_t:
        case SDLK_u:
        case SDLK_v:
        case SDLK_w:
        case SDLK_x:
        case SDLK_y:
        case SDLK_z:
        {
            const int alpha = event->key.keysym.sym - SDLK_a;
            const enum option option = (enum option)(alpha + 1);
            return select_option(option);
        }
        break;
        default:
            break;
        }
    }
    break;
    case SDL_MOUSEBUTTONDOWN:
    {
        if (event->button.button == SDL_BUTTON_LEFT)
        {
            const enum option option = option_mouseover();
            return select_option(option);
        }
    }
    break;
    case SDL_MOUSEMOTION:
    {
        mouse_x = event->motion.x;
        mouse_y = event->motion.y;
    }
    break;
    default:
        break;
    }

    return &menu_scene;
}

static struct scene *update(TCOD_Console *const console, const float delta_time)
{
    int y = 1;

    for (enum option option = 1; option < NUM_OPTIONS; option++)
    {
        console_print(
            console,
            1,
            y++,
            option == option_mouseover() ? &TCOD_yellow : &TCOD_white,
            &TCOD_black,
            TCOD_BKGND_NONE,
            TCOD_LEFT,
            "%c) %s",
            option + 'a' - 1,
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
