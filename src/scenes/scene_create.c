#include "scene_create.h"

#include "../config.h"
#include "../game/actor.h"
#include "../game/world.h"
#include "../print.h"
#include "../scene.h"
#include "scene_game.h"
#include "scene_menu.h"
#include <libtcod.h>

struct actor *hero;

static void init(struct scene *previous_scene)
{
    world_init();

    hero = actor_new("Blinky", RACE_HUMAN, CLASS_WARRIOR, FACTION_ADVENTURER, 1, 0, 0, 0, true);
    hero->gold = 100;

    struct item *bodkin_arrow = item_new(ITEM_TYPE_BODKIN_ARROW, 0, 0, 0, 50);
    TCOD_list_push(hero->items, bodkin_arrow);
    struct item *bolt = item_new(ITEM_TYPE_BOLT, 0, 0, 0, 50);
    TCOD_list_push(hero->items, bolt);
    struct item *cold_iron_blade = item_new(ITEM_TYPE_COLD_IRON_BLADE, 0, 0, 0, 1);
    TCOD_list_push(hero->items, cold_iron_blade);
    struct item *crossbow = item_new(ITEM_TYPE_CROSSBOW, 0, 0, 0, 1);
    TCOD_list_push(hero->items, crossbow);
    struct item *iron_armor = item_new(ITEM_TYPE_IRON_ARMOR, 0, 0, 0, 1);
    TCOD_list_push(hero->items, iron_armor);
    struct item *greatsword = item_new(ITEM_TYPE_GREATSWORD, 0, 0, 0, 1);
    TCOD_list_push(hero->items, greatsword);
    struct item *healing_potion = item_new(ITEM_TYPE_HEALING_POTION, 0, 0, 0, 10);
    TCOD_list_push(hero->items, healing_potion);
    struct item *kite_shield = item_new(ITEM_TYPE_KITE_SHIELD, 0, 0, 0, 1);
    TCOD_list_push(hero->items, kite_shield);
    struct item *longbow = item_new(ITEM_TYPE_LONGBOW, 0, 0, 0, 1);
    TCOD_list_push(hero->items, longbow);
    struct item *longsword = item_new(ITEM_TYPE_LONGSWORD, 0, 0, 0, 1);
    TCOD_list_push(hero->items, longsword);
    struct item *scepter_of_unity = item_new(ITEM_TYPE_SCEPTER_OF_UNITY, 0, 0, 0, 1);
    TCOD_list_push(hero->items, scepter_of_unity);
    struct item *spiked_shield = item_new(ITEM_TYPE_SPIKED_SHIELD, 0, 0, 0, 1);
    TCOD_list_push(hero->items, spiked_shield);
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
