#include "create.h"

#include "../config.h"
#include "../game/actor.h"
#include "../game/world.h"
#include "../print.h"
#include "../scene.h"
#include "game.h"
#include "menu.h"
#include <libtcod.h>

struct actor *hero;

static void init(struct scene *previous_scene)
{
    world_init();

    hero = actor_new("Jalina Feynolt", RACE_HUMAN, CLASS_WARRIOR, FACTION_ADVENTURER, 1, 0, 0, 0, true);
    hero->gold = 100;

    hero->equipment[EQUIP_SLOT_WEAPON] = item_new(ITEM_TYPE_LONGSWORD_1, 0, 0, 0, 1);
    hero->equipment[EQUIP_SLOT_SHIELD] = item_new(ITEM_TYPE_TOWER_SHIELD_1, 0, 0, 0, 1);
    hero->equipment[EQUIP_SLOT_ARMOR] = item_new(ITEM_TYPE_FULL_PLATE_1, 0, 0, 0, 1);
    hero->equipment[EQUIP_SLOT_AMMUNITION] = item_new(ITEM_TYPE_ARROW_1, 0, 0, 0, 50);
    TCOD_list_push(hero->items, item_new(ITEM_TYPE_LONGBOW_1, 0, 0, 0, 1));
    TCOD_list_push(hero->items, item_new(ITEM_TYPE_POTION_MINOR_HEAL, 0, 0, 0, 10));
    TCOD_list_push(hero->items, item_new(ITEM_TYPE_SCROLL_LIGHTNING, 0, 0, 0, 10));
    TCOD_list_push(hero->known_spell_types, (void *)SPELL_TYPE_MINOR_HEAL);
    hero->readied_spell_type = SPELL_TYPE_MINOR_HEAL;
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
            world_create(hero);

            create_scene.uninit();
            game_scene.init(&create_scene);
            return &game_scene;
        }
        break;
        case SDLK_ESCAPE:
        {
            actor_delete(hero);

            world_uninit();

            create_scene.uninit();
            menu_scene.init(&create_scene);
            return &menu_scene;
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
            world_create(hero);

            create_scene.uninit();
            game_scene.init(&create_scene);
            return &game_scene;
        }
        else if (event->button.button == SDL_BUTTON_RIGHT)
        {
            actor_delete(hero);

            world_uninit();

            create_scene.uninit();
            menu_scene.init(&create_scene);
            return &menu_scene;
        }
    }
    break;
    default:
        break;
    }

    return &create_scene;
}

static struct scene *update(TCOD_Console *const console, const float delta_time)
{
    int y = 1;
    console_print(
        console,
        1,
        y++,
        &TCOD_white,
        &TCOD_black,
        TCOD_BKGND_NONE,
        TCOD_LEFT,
        "TODO: character creation.");
    console_print(
        console,
        1,
        y++,
        &TCOD_white,
        &TCOD_black,
        TCOD_BKGND_NONE,
        TCOD_LEFT,
        "Press ENTER or L-Mouse to start.");
    console_print(
        console,
        1,
        y++,
        &TCOD_white,
        &TCOD_black,
        TCOD_BKGND_NONE,
        TCOD_LEFT,
        "Press ESC or R-Mouse to return.");

    TCOD_console_printn_frame(
        console,
        0, 0, console_width, console_height,
        sizeof(TITLE),
        TITLE,
        &TCOD_white,
        &TCOD_black,
        TCOD_BKGND_NONE,
        false);

    return &create_scene;
}

struct scene create_scene =
    {&init,
     &uninit,
     &handle_event,
     &update};
