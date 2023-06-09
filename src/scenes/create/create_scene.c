#include "create_scene.h"

#include "../../config.h"
#include "../../game/actor.h"
#include "../../game/color.h"
#include "../../game/world.h"
#include "../../print.h"
#include "../../scene.h"
#include "../game/game_scene.h"
#include "../menu/menu_scene.h"
#include <libtcod.h>

struct actor *hero;

static struct scene *start(void)
{
    world_create(hero);

    create_scene.uninit();
    game_scene.init(&create_scene);
    return &game_scene;
}

static struct scene *back(void)
{
    actor_delete(hero);

    world_uninit();

    create_scene.uninit();
    menu_scene.init(&create_scene);
    return &menu_scene;
}

static void init(struct scene *previous_scene)
{
    previous_scene;

    world_init();

    hero = actor_new(
        "Jalina Feynolt",
        RACE_HUMAN,
        CLASS_FIGHTER,
        FACTION_ADVENTURER,
        0,
        0,
        0);

    hero->equipment[EQUIP_SLOT_WEAPON] = item_new(ITEM_TYPE_LONGSWORD, 0, 0, 0, 1);
    list_add(hero->known_spell_types, (void *)SPELL_TYPE_MINOR_HEAL);
    hero->readied_spell_type = SPELL_TYPE_MINOR_HEAL;

    hero->light_type = LIGHT_TYPE_TORCH;
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
        case SDLK_RETURN:
        {
            return start();
        }
        break;
        case SDLK_ESCAPE:
        {
            return back();
        }
        break;
        }
    }
    break;
    }

    return &create_scene;
}

static struct scene *update(TCOD_Console *const console, const float delta_time)
{
    delta_time;

    int y = 1;

    console_print(
        console,
        1,
        y++,
        &color_white,
        &color_black,
        TCOD_BKGND_NONE,
        TCOD_LEFT,
        "TODO: character creation.");

    console_print(
        console,
        1,
        y++,
        &color_white,
        &color_black,
        TCOD_BKGND_NONE,
        TCOD_LEFT,
        "Press ENTER to start.");

    console_print(
        console,
        1,
        y++,
        &color_white,
        &color_black,
        TCOD_BKGND_NONE,
        TCOD_LEFT,
        "Press ESC to return.");

    TCOD_console_printn_frame(
        console,
        0, 0, console_width, console_height,
        sizeof(TITLE),
        TITLE,
        &color_white,
        &color_black,
        TCOD_BKGND_NONE,
        false);

    return &create_scene;
}

struct scene create_scene =
    {&init,
     &uninit,
     &handle_event,
     &update};
