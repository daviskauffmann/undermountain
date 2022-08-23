#include "scene_create.h"

#include "../config.h"
#include "../game/actor.h"
#include "../game/world.h"
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

static struct scene *handle_event(TCOD_event_t ev, TCOD_key_t key, TCOD_mouse_t mouse)
{
    switch (ev)
    {
    case TCOD_EVENT_KEY_PRESS:
    {
        switch (key.vk)
        {
        case TCODK_ENTER:
        {
            world_create(hero);

            create_scene.quit();
            game_scene.init(&create_scene);
            return &game_scene;
        }
        break;
        case TCODK_ESCAPE:
        {
            actor_delete(hero);

            world_cleanup();

            create_scene.quit();
            menu_scene.init(&create_scene);
            return &menu_scene;
        }
        break;
        default:
            break;
        }
    }
    break;
    case TCOD_EVENT_MOUSE_PRESS:
    {
        if (mouse.lbutton)
        {
            world_create(hero);

            create_scene.quit();
            game_scene.init(&create_scene);
            return &game_scene;
        }
        else if (mouse.rbutton)
        {
            actor_delete(hero);

            world_cleanup();

            create_scene.quit();
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

static struct scene *update(float delta_time)
{
    return &create_scene;
}

static void render(TCOD_console_t console)
{
    int y = 1;
    TCOD_console_printf_rect(
        console,
        1,
        y++,
        console_width - 2,
        console_height - 2,
        "TODO: character creation.");
    TCOD_console_printf_rect(
        console,
        1,
        y++,
        console_width - 2,
        console_height - 2,
        "Press ENTER or L-Mouse to start.");
    TCOD_console_printf_rect(
        console,
        1,
        y++,
        console_width - 2,
        console_height - 2,
        "Press ESC or R-Mouse to return.");

    TCOD_console_set_default_foreground(console, TCOD_white);
    TCOD_console_printf_frame(
        console,
        0,
        0,
        console_width,
        console_height,
        false,
        TCOD_BKGND_SET,
        TITLE);
}

static void quit(void)
{
}

struct scene create_scene =
    {&init,
     &handle_event,
     &update,
     &render,
     &quit};
