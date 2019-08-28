#include "scene_game.h"

#include <assert.h>
#include <float.h>
#include <malloc.h>
#include <math.h>
#include <stdio.h>
#include <string.h>

#include <libtcod.h>

#include "scene_menu.h"
#include "../config.h"
#include "../scene.h"
#include "../sys.h"

#include "../../game/actor.h"
#include "../../game/assets.h"
#include "../../game/item.h"
#include "../../game/message.h"
#include "../../game/projectile.h"
#include "../../game/util.h"
#include "../../game/world.h"

// TODO: better mouse controls
// design a system where when selecting an action from the right-click menu, other actions are ignored as the actor paths to the location
// when left-clicking a hostile actor, or selecting Attack from the menu, should track that actor's position and follow it. when reached, attack once and stop automoving

// TODO: automoving needs to be slower and easy to cancel

// TODO: better sanity checking for tooltip_rect data
// right now, tooltip_rect data is contained in a module scope struct
// need to define ways to make sure the struct is valid for the given tooltip_rect options

// TODO: prompts

static int mouse_x;
static int mouse_y;
static int mouse_tile_x;
static int mouse_tile_y;

static bool automoving;
static int automove_x;
static int automove_y;
static struct actor *automove_actor;

static bool took_turn;

enum directional_action
{
    DIRECTIONAL_ACTION_NONE,
    DIRECTIONAL_ACTION_CLOSE_DOOR,
    DIRECTIONAL_ACTION_DRINK,
    DIRECTIONAL_ACTION_OPEN_CHEST,
    DIRECTIONAL_ACTION_OPEN_DOOR,
    DIRECTIONAL_ACTION_PRAY,
    DIRECTIONAL_ACTION_SIT
};

static enum directional_action directional_action;

static bool do_directional_action(struct actor *player, int x, int y)
{
    bool success = false;

    switch (directional_action)
    {
    case DIRECTIONAL_ACTION_CLOSE_DOOR:
    {
        success = actor_close_door(player, x, y);
    }
    break;
    case DIRECTIONAL_ACTION_DRINK:
    {
        success = actor_drink(player, x, y);
    }
    break;
    case DIRECTIONAL_ACTION_OPEN_CHEST:
    {
        success = actor_open_chest(player, x, y);
    }
    break;
    case DIRECTIONAL_ACTION_OPEN_DOOR:
    {
        success = actor_open_door(player, x, y);
    }
    break;
    case DIRECTIONAL_ACTION_PRAY:
    {
        success = actor_pray(player, x, y);
    }
    break;
    case DIRECTIONAL_ACTION_SIT:
    {
        success = actor_sit(player, x, y);
    }
    break;
    }

    directional_action = DIRECTIONAL_ACTION_NONE;

    return success;
}

enum inventory_action
{
    INVENTORY_ACTION_NONE,
    INVENTORY_ACTION_DROP,
    INVENTORY_ACTION_EQUIP,
    INVENTORY_ACTION_EXAMINE,
    INVENTORY_ACTION_QUAFF
};

static enum inventory_action inventory_action;

enum character_action
{
    CHARACTER_ACTION_NONE,
    CHARACTER_ACTION_EXAMINE,
    CHARACTER_ACTION_UNEQUIP
};

static enum character_action character_action;

enum targeting
{
    TARGETING_NONE,
    TARGETING_LOOK,
    TARGETING_EXAMINE,
    TARGETING_SHOOT,
    TARGETING_SPELL
};

static enum targeting targeting;
static int target_x;
static int target_y;

static int view_x;
static int view_y;
static int view_width;
static int view_height;

static bool view_is_inside(int x, int y)
{
    return x >= 0 &&
           x < view_width &&
           y >= 0 &&
           y < view_height;
}

static TCOD_noise_t noise;
static float noise_x;

struct rect
{
    TCOD_console_t console;
    bool visible;
    int x;
    int y;
    int width;
    int height;
};

static bool rect_is_inside(struct rect rect, int x, int y)
{
    return rect.visible &&
           x >= rect.x &&
           x < rect.x + rect.width &&
           y >= rect.y &&
           y < rect.y + rect.height;
}

static struct rect message_log_rect;

enum panel
{
    PANEL_CHARACTER,
    PANEL_EXAMINE,
    PANEL_INVENTORY,
    PANEL_SPELLBOOK,

    NUM_PANELS
};

struct panel_state
{
    int scroll;
    bool selection_mode;
};

static struct rect panel_rect;
static struct panel_state panel_state[NUM_PANELS];
static enum panel current_panel;

static void panel_toggle(enum panel panel)
{
    if (panel_rect.visible)
    {
        if (current_panel == panel)
        {
            panel_rect.visible = false;
        }
        else
        {
            current_panel = panel;
        }
    }
    else
    {
        current_panel = panel;
        panel_rect.visible = true;
    }
}

static void panel_show(enum panel panel)
{
    if (!panel_rect.visible || current_panel != panel)
    {
        panel_toggle(panel);
    }
}

static enum equip_slot panel_character_get_selected(void)
{
    if (panel_rect.visible && current_panel == PANEL_CHARACTER)
    {
        int y = 15;
        for (enum equip_slot equip_slot = 1; equip_slot < NUM_EQUIP_SLOTS; equip_slot++)
        {
            if (mouse_x > panel_rect.x &&
                mouse_x < panel_rect.x + panel_rect.width &&
                mouse_y == y + panel_rect.y - panel_state[current_panel].scroll)
            {
                return equip_slot;
            }
            y++;
        }
    }
    return -1;
}

static struct item *panel_inventory_get_selected(void)
{
    if (panel_rect.visible && current_panel == PANEL_INVENTORY)
    {
        int y = 1;
        TCOD_LIST_FOREACH(world->player->items)
        {
            struct item *item = *iterator;
            if (mouse_x > panel_rect.x &&
                mouse_x < panel_rect.x + panel_rect.width &&
                mouse_y == y + panel_rect.y - panel_state[current_panel].scroll)
            {
                return item;
            }
            y++;
        }
    }
    return NULL;
}

struct tooltip_data
{
    int x;
    int y;
    struct object *object;
    struct item *item;
    struct actor *actor;
    enum equip_slot equip_slot;
};

struct tooltip_option
{
    char *text;
    bool (*on_click)(void);
};

static struct rect tooltip_rect;
static TCOD_list_t tooltip_options;
static struct tooltip_data tooltip_data;

struct tooltip_option *tooltip_option_new(char *text, bool (*on_click)(void))
{
    struct tooltip_option *tooltip_option = malloc(sizeof(struct tooltip_option));
    assert(tooltip_option);
    tooltip_option->text = _strdup(text);
    tooltip_option->on_click = on_click;
    return tooltip_option;
}

void tooltip_option_delete(struct tooltip_option *tooltip_option)
{
    free(tooltip_option->text);
    free(tooltip_option);
}

static void tooltip_options_add(char *text, bool (*on_click)(void))
{
    struct tooltip_option *tooltip_option = tooltip_option_new(text, on_click);
    TCOD_list_push(tooltip_options, tooltip_option);
}

static void tooltip_options_clear(void)
{
    TCOD_LIST_FOREACH(tooltip_options)
    {
        struct tooltip_option *tooltip_option = *iterator;
        iterator = TCOD_list_remove_iterator(tooltip_options, iterator);
        tooltip_option_delete(tooltip_option);
    }
}

static void tooltip_show(void)
{
    tooltip_options_clear();
    tooltip_rect.visible = true;
    tooltip_rect.x = mouse_x;
    tooltip_rect.y = mouse_y;
}

static void tooltip_hide(void)
{
    tooltip_options_clear();
    tooltip_rect.visible = false;
}

static struct tooltip_option *tooltip_get_selected(void)
{
    if (tooltip_rect.visible)
    {
        int y = 1;
        TCOD_LIST_FOREACH(tooltip_options)
        {
            struct tooltip_option *option = *iterator;
            if (mouse_x > tooltip_rect.x &&
                mouse_x < tooltip_rect.x + tooltip_rect.width &&
                mouse_y == y + tooltip_rect.y)
            {
                return option;
            }
            y++;
        }
    }
    return NULL;
}

static void init(struct scene *previous_scene)
{
    took_turn = true;

    message_log_rect.console = TCOD_console_new(console_width, console_height);
    message_log_rect.visible = true;

    panel_rect.console = TCOD_console_new(console_width, console_height);
    current_panel = PANEL_CHARACTER;

    tooltip_rect.console = TCOD_console_new(console_width, console_height);
    tooltip_options = TCOD_list_new();

    noise = TCOD_noise_new(1, TCOD_NOISE_DEFAULT_HURST, TCOD_NOISE_DEFAULT_LACUNARITY, NULL);
}

static void projectile_on_hit_set_took_turn(void *on_hit_params)
{
    took_turn = true;
}

static bool toolip_option_on_click_move(void)
{
    automoving = true;

    struct tile *tile = &world->maps[world->player->floor].tiles[tooltip_data.x][tooltip_data.y];

    if (tile->actor && tile->actor->faction != world->player->faction && !tile->actor->dead)
    {
        automove_actor = tile->actor;
    }
    else
    {
        automove_x = tooltip_data.x;
        automove_y = tooltip_data.y;
    }

    return false;
}

static struct scene *handle_event(TCOD_event_t ev, TCOD_key_t key, TCOD_mouse_t mouse)
{
    mouse_x = mouse.cx;
    mouse_y = mouse.cy;
    mouse_tile_x = mouse.cx + view_x;
    mouse_tile_y = mouse.cy + view_y;

    switch (ev)
    {
    case TCOD_EVENT_KEY_PRESS:
    {
        automoving = false;

        switch (key.vk)
        {
        case TCODK_ESCAPE:
        {
            if (tooltip_rect.visible)
            {
                tooltip_hide();
            }
            else if (directional_action != DIRECTIONAL_ACTION_NONE ||
                     inventory_action != INVENTORY_ACTION_NONE ||
                     character_action != CHARACTER_ACTION_NONE)
            {
                directional_action = DIRECTIONAL_ACTION_NONE;
                inventory_action = INVENTORY_ACTION_NONE;
                character_action = CHARACTER_ACTION_NONE;
                for (enum panel panel = 0; panel < NUM_PANELS; panel++)
                {
                    panel_state[panel].selection_mode = false;
                }
            }
            else if (panel_rect.visible)
            {
                panel_rect.visible = false;
            }
            else if (targeting)
            {
                targeting = false;
            }
            else
            {
                game_scene.quit();
                menu_scene.init(&game_scene);
                return &menu_scene;
            }
        }
        break;
        case TCODK_PAGEDOWN:
        {
            if (panel_rect.visible)
            {
                panel_state[current_panel].scroll++;
            }
        }
        break;
        case TCODK_PAGEUP:
        {
            if (panel_rect.visible)
            {
                panel_state[current_panel].scroll--;
            }
        }
        break;
        case TCODK_KP1:
        {
            if (targeting != TARGETING_NONE)
            {
                target_x--;
                target_y++;
            }
            else if (world->state == WORLD_STATE_PLAY)
            {
                int x = world->player->x - 1;
                int y = world->player->y + 1;
                if (directional_action == DIRECTIONAL_ACTION_NONE)
                {
                    if (key.lctrl)
                    {
                        took_turn = actor_swing(world->player, x, y);
                    }
                    else
                    {
                        took_turn = actor_move(world->player, x, y);
                    }
                }
                else
                {
                    took_turn = do_directional_action(world->player, x, y);
                }
            }
        }
        break;
        case TCODK_KP2:
        case TCODK_DOWN:
        {
            if (targeting != TARGETING_NONE)
            {
                target_x;
                target_y++;
            }
            else if (world->state == WORLD_STATE_PLAY)
            {
                int x = world->player->x;
                int y = world->player->y + 1;
                if (directional_action == DIRECTIONAL_ACTION_NONE)
                {
                    if (key.lctrl)
                    {
                        took_turn = actor_swing(world->player, x, y);
                    }
                    else
                    {
                        took_turn = actor_move(world->player, x, y);
                    }
                }
                else
                {
                    took_turn = do_directional_action(world->player, x, y);
                }
            }
        }
        break;
        case TCODK_KP3:
        {
            if (targeting != TARGETING_NONE)
            {
                target_x++;
                target_y++;
            }
            else if (world->state == WORLD_STATE_PLAY)
            {
                int x = world->player->x + 1;
                int y = world->player->y + 1;
                if (directional_action == DIRECTIONAL_ACTION_NONE)
                {
                    if (key.lctrl)
                    {
                        took_turn = actor_swing(world->player, x, y);
                    }
                    else
                    {
                        took_turn = actor_move(world->player, x, y);
                    }
                }
                else
                {
                    took_turn = do_directional_action(world->player, x, y);
                }
            }
        }
        break;
        case TCODK_KP4:
        case TCODK_LEFT:
        {
            if (targeting != TARGETING_NONE)
            {
                target_x--;
                target_y;
            }
            else if (world->state == WORLD_STATE_PLAY)
            {
                int x = world->player->x - 1;
                int y = world->player->y;
                if (directional_action == DIRECTIONAL_ACTION_NONE)
                {
                    if (key.lctrl)
                    {
                        took_turn = actor_swing(world->player, x, y);
                    }
                    else
                    {
                        took_turn = actor_move(world->player, x, y);
                    }
                }
                else
                {
                    took_turn = do_directional_action(world->player, x, y);
                }
            }
        }
        break;
        case TCODK_KP5:
        {
            if (world->state != WORLD_STATE_WAIT)
            {
                took_turn = true;
            }
        }
        break;
        case TCODK_KP6:
        case TCODK_RIGHT:
        {
            if (targeting != TARGETING_NONE)
            {
                target_x++;
                target_y;
            }
            else if (world->state == WORLD_STATE_PLAY)
            {
                int x = world->player->x + 1;
                int y = world->player->y;
                if (directional_action == DIRECTIONAL_ACTION_NONE)
                {
                    if (key.lctrl)
                    {
                        took_turn = actor_swing(world->player, x, y);
                    }
                    else
                    {
                        took_turn = actor_move(world->player, x, y);
                    }
                }
                else
                {
                    took_turn = do_directional_action(world->player, x, y);
                }
            }
        }
        break;
        case TCODK_KP7:
        {
            if (targeting != TARGETING_NONE)
            {
                target_x--;
                target_y--;
            }
            else if (world->state == WORLD_STATE_PLAY)
            {
                int x = world->player->x - 1;
                int y = world->player->y - 1;
                if (directional_action == DIRECTIONAL_ACTION_NONE)
                {
                    if (key.lctrl)
                    {
                        took_turn = actor_swing(world->player, x, y);
                    }
                    else
                    {
                        took_turn = actor_move(world->player, x, y);
                    }
                }
                else
                {
                    took_turn = do_directional_action(world->player, x, y);
                }
            }
        }
        break;
        case TCODK_KP8:
        case TCODK_UP:
        {
            if (targeting != TARGETING_NONE)
            {
                target_x;
                target_y--;
            }
            else if (world->state == WORLD_STATE_PLAY)
            {
                int x = world->player->x;
                int y = world->player->y - 1;
                if (directional_action == DIRECTIONAL_ACTION_NONE)
                {
                    if (key.lctrl)
                    {
                        took_turn = actor_swing(world->player, x, y);
                    }
                    else
                    {
                        took_turn = actor_move(world->player, x, y);
                    }
                }
                else
                {
                    took_turn = do_directional_action(world->player, x, y);
                }
            }
        }
        break;
        case TCODK_KP9:
        {
            if (targeting != TARGETING_NONE)
            {
                target_x++;
                target_y--;
            }
            else if (world->state == WORLD_STATE_PLAY)
            {
                int x = world->player->x + 1;
                int y = world->player->y - 1;
                if (directional_action == DIRECTIONAL_ACTION_NONE)
                {
                    if (key.lctrl)
                    {
                        took_turn = actor_swing(world->player, x, y);
                    }
                    else
                    {
                        took_turn = actor_move(world->player, x, y);
                    }
                }
                else
                {
                    took_turn = do_directional_action(world->player, x, y);
                }
            }
        }
        break;
        case TCODK_TEXT:
        {
            bool handled = false;
            int alpha = key.text[0] - 'a';
            if (inventory_action != INVENTORY_ACTION_NONE && alpha >= 0 && alpha < TCOD_list_size(world->player->items))
            {
                struct item *item = TCOD_list_get(world->player->items, alpha);
                switch (inventory_action)
                {
                case INVENTORY_ACTION_EQUIP:
                {
                    if (world->state == WORLD_STATE_PLAY)
                    {
                        took_turn = actor_equip(world->player, item);
                    }
                }
                break;
                case INVENTORY_ACTION_EXAMINE:
                {
                    // TODO: send examine target to ui
                    panel_show(PANEL_EXAMINE);
                }
                break;
                case INVENTORY_ACTION_DROP:
                {
                    if (world->state == WORLD_STATE_PLAY)
                    {
                        took_turn = actor_drop(world->player, item);
                    }
                }
                break;
                case INVENTORY_ACTION_QUAFF:
                {
                    if (world->state == WORLD_STATE_PLAY)
                    {
                        took_turn = actor_quaff(world->player, item);
                    }
                }
                break;
                }

                inventory_action = INVENTORY_ACTION_NONE;
                panel_state[PANEL_INVENTORY].selection_mode = false;

                handled = true;
            }
            else if (character_action != CHARACTER_ACTION_NONE && alpha >= 0 && alpha < NUM_EQUIP_SLOTS - 1)
            {
                enum equip_slot equip_slot = (enum equip_slot)(alpha + 1);
                switch (character_action)
                {
                case CHARACTER_ACTION_EXAMINE:
                {
                    // TODO: send examine target to ui

                    panel_show(PANEL_EXAMINE);
                }
                break;
                case CHARACTER_ACTION_UNEQUIP:
                {
                    if (world->state == WORLD_STATE_PLAY)
                    {
                        took_turn = actor_unequip(world->player, equip_slot);
                    }
                }
                break;
                }

                character_action = CHARACTER_ACTION_NONE;
                panel_state[PANEL_CHARACTER].selection_mode = false;

                handled = true;
            }

            for (enum panel panel = 0; panel < NUM_PANELS; panel++)
            {
                if (panel_state[panel].selection_mode)
                {
                    handled = true;
                }
            }

            if (handled)
            {
                break;
            }

            switch (key.text[0])
            {
            case '<':
            {
                if (world->state == WORLD_STATE_PLAY)
                {
                    took_turn = actor_ascend(world->player, false);
                }
            }
            break;
            case '>':
            {
                if (world->state == WORLD_STATE_PLAY)
                {
                    took_turn = actor_descend(world->player, false);
                }
            }
            break;
            case 'b':
            {
                panel_toggle(PANEL_SPELLBOOK);
            }
            break;
            case 'C':
            {
                panel_toggle(PANEL_CHARACTER);
            }
            break;
            case 'c':
            {
                if (world->state == WORLD_STATE_PLAY)
                {
                    directional_action = DIRECTIONAL_ACTION_CLOSE_DOOR;

                    world_log(
                        world->player->floor,
                        world->player->x,
                        world->player->y,
                        TCOD_white,
                        "Choose a direction, ESC to cancel");
                }
            }
            break;
            case 'D':
            {
                if (world->state == WORLD_STATE_PLAY)
                {
                    directional_action = DIRECTIONAL_ACTION_DRINK;

                    world_log(
                        world->player->floor,
                        world->player->x,
                        world->player->y,
                        TCOD_white,
                        "Choose a direction, ESC to cancel");
                }
            }
            break;
            case 'd':
            {
                if (world->state == WORLD_STATE_PLAY)
                {
                    panel_show(PANEL_INVENTORY);
                    inventory_action = INVENTORY_ACTION_DROP;
                    panel_state[PANEL_INVENTORY].selection_mode = true;

                    world_log(
                        world->player->floor,
                        world->player->x,
                        world->player->y,
                        TCOD_white,
                        "Choose an item to drop, ESC to cancel");
                }
            }
            break;
            case 'e':
            {
                if (world->state == WORLD_STATE_PLAY)
                {
                    panel_show(PANEL_INVENTORY);
                    inventory_action = INVENTORY_ACTION_EQUIP;
                    panel_state[PANEL_INVENTORY].selection_mode = true;

                    world_log(
                        world->player->floor,
                        world->player->x,
                        world->player->y,
                        TCOD_white,
                        "Choose an item to equip, ESC to cancel");
                }
            }
            break;
            case 'f':
            {
                if (world->state == WORLD_STATE_PLAY)
                {
                    if (targeting == TARGETING_SHOOT)
                    {
                        actor_shoot(world->player, target_x, target_y, &projectile_on_hit_set_took_turn, NULL);
                        targeting = TARGETING_NONE;
                    }
                    else
                    {
                        targeting = TARGETING_SHOOT;
                        struct actor *target = world->player;
                        struct map *map = &world->maps[world->player->floor];
                        float min_distance = FLT_MAX;
                        TCOD_LIST_FOREACH(map->actors)
                        {
                            struct actor *actor = *iterator;
                            if (TCOD_map_is_in_fov(world->player->fov, actor->x, actor->y) &&
                                actor->faction != world->player->faction &&
                                !actor->dead)
                            {
                                float distance = distance_between_sq(world->player->x, world->player->y, actor->x, actor->y);
                                if (distance < min_distance)
                                {
                                    target = actor;
                                    min_distance = distance;
                                }
                            }
                        }
                        target_x = target->x;
                        target_y = target->y;
                    }
                }
            }
            break;
            case 'g':
            {
                if (world->state == WORLD_STATE_PLAY)
                {
                    took_turn = actor_grab(world->player, world->player->x, world->player->y);
                }
            }
            break;
            case 'i':
            {
                panel_toggle(PANEL_INVENTORY);
            }
            break;
            case 'l':
            {
                if (targeting == TARGETING_LOOK)
                {
                    targeting = TARGETING_NONE;
                }
                else
                {
                    targeting = TARGETING_LOOK;
                    target_x = world->player->x;
                    target_y = world->player->y;
                }
            }
            break;
            case 'm':
            {
                message_log_rect.visible = !message_log_rect.visible;
            }
            break;
            case 'O':
            {
                if (world->state == WORLD_STATE_PLAY)
                {
                    directional_action = DIRECTIONAL_ACTION_OPEN_CHEST;

                    world_log(
                        world->player->floor,
                        world->player->x,
                        world->player->y,
                        TCOD_white,
                        "Choose a direction, ESC to cancel");
                }
            }
            break;
            case 'o':
            {
                if (world->state == WORLD_STATE_PLAY)
                {
                    directional_action = DIRECTIONAL_ACTION_OPEN_DOOR;

                    world_log(
                        world->player->floor,
                        world->player->x,
                        world->player->y,
                        TCOD_white,
                        "Choose a direction, ESC to cancel");
                }
            }
            break;
            case 'p':
            {
                if (world->state == WORLD_STATE_PLAY)
                {
                    directional_action = DIRECTIONAL_ACTION_PRAY;

                    world_log(
                        world->player->floor,
                        world->player->x,
                        world->player->y,
                        TCOD_white,
                        "Choose a direction, ESC to cancel");
                }
            }
            break;
            case 'q':
            {
                if (world->state == WORLD_STATE_PLAY)
                {
                    panel_show(PANEL_INVENTORY);

                    inventory_action = INVENTORY_ACTION_QUAFF;
                    panel_state[PANEL_INVENTORY].selection_mode = true;

                    world_log(
                        world->player->floor,
                        world->player->x,
                        world->player->y,
                        TCOD_white,
                        "Choose an item to quaff, ESC to cancel");
                }
            }
            break;
            case 's':
            {
                if (world->state == WORLD_STATE_PLAY)
                {
                    directional_action = DIRECTIONAL_ACTION_SIT;

                    world_log(
                        world->player->floor,
                        world->player->x,
                        world->player->y,
                        TCOD_white,
                        "Choose a direction, ESC to cancel");
                }
            }
            break;
            case 't':
            {
                if (world->state == WORLD_STATE_PLAY)
                {
                    world->player->torch = !world->player->torch;
                    took_turn = true;
                }
            }
            break;
            case 'u':
            {
                if (world->state == WORLD_STATE_PLAY)
                {
                    panel_show(PANEL_CHARACTER);
                    character_action = CHARACTER_ACTION_UNEQUIP;
                    panel_state[PANEL_CHARACTER].selection_mode = true;

                    world_log(
                        world->player->floor,
                        world->player->x,
                        world->player->y,
                        TCOD_white,
                        "Choose an item to unequip, ESC to cancel");
                }
            }
            break;
            case 'X':
            {
                panel_show(PANEL_INVENTORY);
                inventory_action = INVENTORY_ACTION_EXAMINE;
                panel_state[PANEL_INVENTORY].selection_mode = true;

                world_log(
                    world->player->floor,
                    world->player->x,
                    world->player->y,
                    TCOD_white,
                    "Choose an item to examine, ESC to cancel");
            }
            break;
            case 'x':
            {
                if (key.lctrl)
                {
                    panel_show(PANEL_CHARACTER);
                    character_action = CHARACTER_ACTION_EXAMINE;
                    panel_state[PANEL_CHARACTER].selection_mode = true;

                    world_log(
                        world->player->floor,
                        world->player->x,
                        world->player->y,
                        TCOD_white,
                        "Choose an equipment to examine, ESC to cancel");
                }
                else
                {
                    if (targeting == TARGETING_EXAMINE)
                    {
                        panel_show(PANEL_EXAMINE); // TODO: send examine target to ui
                        targeting = TARGETING_NONE;
                    }
                    else
                    {
                        targeting = TARGETING_EXAMINE;
                        target_x = world->player->x;
                        target_y = world->player->y;
                    }
                }
            }
            break;
            case 'z':
            {
                if (world->state == WORLD_STATE_PLAY)
                {
                    if (targeting == TARGETING_SPELL)
                    {
                        actor_cast_spell(world->player, target_x, target_y);
                        targeting = TARGETING_NONE;
                    }
                    else
                    {
                        targeting = TARGETING_SPELL;
                        target_x = world->player->x;
                        target_y = world->player->y;
                    }
                }
            }
            break;
            }
        }
        break;
        }
    }
    break;
    case TCOD_EVENT_MOUSE_PRESS:
    {
        if (mouse.lbutton)
        {
            automoving = false;

            if (tooltip_rect.visible)
            {
                if (rect_is_inside(tooltip_rect, mouse_x, mouse_y))
                {
                    struct tooltip_option *tooltip_option = tooltip_get_selected();
                    if (tooltip_option)
                    {
                        if (tooltip_option->on_click)
                        {
                            took_turn = tooltip_option->on_click();
                        }

                        tooltip_hide();
                    }
                }
                else
                {
                    tooltip_hide();
                }
            }
            else if (view_is_inside(mouse_x, mouse_y))
            {
                automoving = true;
                struct tile *tile = &world->maps[world->player->floor].tiles[mouse_tile_x][mouse_tile_y];
                if (tile->actor && tile->actor->faction != world->player->faction && !tile->actor->dead)
                {
                    automove_actor = tile->actor;
                }
                else
                {
                    automove_x = mouse_tile_x;
                    automove_y = mouse_tile_y;
                }
            }
            else if (rect_is_inside(panel_rect, mouse_x, mouse_y))
            {
                if (inventory_action != INVENTORY_ACTION_NONE)
                {
                    struct item *item = panel_inventory_get_selected();
                    if (item)
                    {
                        switch (inventory_action)
                        {
                        case INVENTORY_ACTION_EQUIP:
                        {
                            if (world->state == WORLD_STATE_PLAY)
                            {
                                took_turn = actor_equip(world->player, item);
                            }
                        }
                        break;
                        case INVENTORY_ACTION_DROP:
                        {
                            if (world->state == WORLD_STATE_PLAY)
                            {
                                took_turn = actor_drop(world->player, item);
                            }
                        }
                        break;
                        }

                        inventory_action = INVENTORY_ACTION_NONE;
                        panel_state[PANEL_INVENTORY].selection_mode = false;
                    }
                }
                else if (character_action != CHARACTER_ACTION_NONE)
                {
                    enum equip_slot equip_slot = panel_character_get_selected();
                    if (equip_slot >= 1 && equip_slot < NUM_EQUIP_SLOTS)
                    {
                        if (world->state == WORLD_STATE_PLAY)
                        {
                            took_turn = actor_unequip(world->player, equip_slot);
                        }
                    }

                    character_action = CHARACTER_ACTION_NONE;
                    panel_state[PANEL_CHARACTER].selection_mode = false;
                }
            }
        }
        else if (mouse.rbutton)
        {
            if (view_is_inside(mouse_x, mouse_y) && map_is_inside(mouse_tile_x, mouse_tile_y))
            {
                struct map *map = &world->maps[world->player->floor];
                struct tile *tile = &map->tiles[mouse_tile_x][mouse_tile_y];
                tooltip_show();
                tooltip_options_add("Move", &toolip_option_on_click_move);
                tooltip_data.x = mouse_tile_x;
                tooltip_data.y = mouse_tile_y;
                if (tile->object)
                {
                    tooltip_options_add("Examine Object", NULL);
                    tooltip_options_add("Interact", NULL);
                    tooltip_options_add("Bash", NULL);
                    tooltip_data.object = tile->object;
                }
                if (tile->actor)
                {
                    tooltip_options_add("Examine Actor", NULL);
                    tooltip_options_add("Talk", NULL);
                    tooltip_options_add("Swap", NULL);
                    tooltip_options_add("Attack", NULL);
                    tooltip_data.actor = tile->actor;
                }
                if (TCOD_list_peek(tile->items))
                {
                    tooltip_options_add("Examine Item", NULL);
                    tooltip_options_add("Take Item", NULL);
                    tooltip_data.item = TCOD_list_peek(tile->items);
                }
                if (TCOD_list_size(tile->items) > 1)
                {
                    tooltip_options_add("Take All", NULL);
                }
                tooltip_options_add("Cancel", NULL);
            }
            else if (rect_is_inside(panel_rect, mouse_x, mouse_y))
            {
                switch (current_panel)
                {
                case PANEL_INVENTORY:
                {
                    struct item *item = panel_inventory_get_selected();
                    if (item)
                    {
                        tooltip_show();
                        tooltip_options_add("Drop", NULL);
                        enum base_item base_item = item_datum[item->type].base_item;
                        if (base_item_datum[base_item].equip_slot != EQUIP_SLOT_NONE)
                        {
                            tooltip_options_add("Equip", NULL);
                        }
                        if (base_item == BASE_ITEM_POTION)
                        {
                            tooltip_options_add("Quaff", NULL);
                        }
                        tooltip_data.item = item;
                        tooltip_options_add("Cancel", NULL);
                    }

                    break;
                }
                break;
                case PANEL_CHARACTER:
                {
                    enum equip_slot equip_slot = panel_character_get_selected();
                    if (equip_slot >= 1 && equip_slot < NUM_EQUIP_SLOTS)
                    {
                        struct item *equipment = world->player->equipment[equip_slot];
                        if (equipment)
                        {
                            tooltip_show();
                            tooltip_options_add("Unequip", NULL);
                            tooltip_data.equip_slot = equip_slot;
                            tooltip_options_add("Cancel", NULL);
                        }
                    }
                }
                break;
                }
            }
        }
        else if (mouse.wheel_down)
        {
            if (panel_rect.visible)
            {
                panel_state[current_panel].scroll++;
            }
        }
        else if (mouse.wheel_up)
        {
            if (panel_rect.visible)
            {
                panel_state[current_panel].scroll--;
            }
        }
    }
    break;
    }

    if (automoving && world->state == WORLD_STATE_PLAY)
    {
        if (automove_actor)
        {
            automove_x = automove_actor->x;
            automove_y = automove_actor->y;
        }

        // TODO: probably shouldnt use the path function for this
        // we need to implement custom behavior depending on what the player is doing
        // for example, if the player selects the interact option on a tooltip_rect for an object far away,
        //      the player should navigate there but not interact/attack anything along the way
        took_turn = actor_path_towards(world->player, automove_x, automove_y);
        automoving = took_turn;
    }
    else
    {
        automove_actor = NULL;
    }

    return &game_scene;
}

static struct scene *update(float delta)
{
    message_log_rect.x = 0;
    message_log_rect.height = console_height / 4;
    message_log_rect.y = console_height - message_log_rect.height;
    message_log_rect.width = console_width;

    panel_rect.width = console_width / 2;
    panel_rect.x = console_width - panel_rect.width;
    panel_rect.y = 0;
    panel_rect.height = console_height - (message_log_rect.visible ? message_log_rect.height : 0);

    tooltip_rect.width = 0;
    TCOD_LIST_FOREACH(tooltip_options)
    {
        struct tooltip_option *option = *iterator;
        int len = (int)strlen(option->text) + 2;
        if (len > tooltip_rect.width)
        {
            tooltip_rect.width = len;
        }
    }
    tooltip_rect.height = TCOD_list_size(tooltip_options) + 2;

    view_width = console_width - (panel_rect.visible ? panel_rect.width : 0);
    view_height = console_height - (message_log_rect.visible ? message_log_rect.height : 0);
    view_x = world->player->x - view_width / 2;
    view_y = world->player->y - view_height / 2;
    if (view_x + view_width > MAP_WIDTH)
    {
        view_x = MAP_WIDTH - view_width;
    }
    if (view_x < 0)
    {
        view_x = 0;
    }
    if (view_y + view_height > MAP_HEIGHT)
    {
        view_y = MAP_HEIGHT - view_height;
    }
    if (view_y < 0)
    {
        view_y = 0;
    }

    world_update();
    if (took_turn)
    {
        world_turn();
        took_turn = false;
    }
    if (world->state == WORLD_STATE_LOSE && file_exists(SAVE_PATH))
    {
        remove(SAVE_PATH);
    }

    return &game_scene;
}

static void render(TCOD_console_t console)
{
    struct map *map = &world->maps[world->player->floor];

    {
        noise_x += 0.2f;
        float noise_dx = noise_x + 20.0f;
        float dx = TCOD_noise_get(noise, &noise_dx) * 0.5f;
        noise_dx += 30.0f;
        float dy = TCOD_noise_get(noise, &noise_dx) * 0.5f;
        float di = 0.2f * TCOD_noise_get(noise, &noise_x);

        for (int x = view_x; x < view_x + view_width; x++)
        {
            for (int y = view_y; y < view_y + view_height; y++)
            {
                if (map_is_inside(x, y))
                {
                    struct tile *tile = &map->tiles[x][y];
                    struct tile_data tile_data = tile_datum[tile->type];
                    TCOD_color_t fg_color = TCOD_color_multiply(
                        tile_common.ambient_color,
                        tile_data.color);
                    TCOD_color_t bg_color = TCOD_color_multiply_scalar(
                        fg_color,
                        tile_common.ambient_intensity);
                    if (TCOD_map_is_in_fov(world->player->fov, x, y))
                    {
                        tile->seen = true;

                        TCOD_LIST_FOREACH(map->actors)
                        {
                            struct actor *actor = *iterator;
                            if (actor->glow_fov && TCOD_map_is_in_fov(actor->glow_fov, x, y))
                            {
                                float radius_sq = powf((float)actor_common.glow_radius, 2);
                                float distance_sq =
                                    powf((float)(x - actor->x), 2) +
                                    powf((float)(y - actor->y), 2);
                                float coef = CLAMP(
                                    0.0f,
                                    1.0f,
                                    (radius_sq - distance_sq) / radius_sq);
                                fg_color = TCOD_color_lerp(
                                    fg_color,
                                    TCOD_color_lerp(
                                        tile_data.color,
                                        actor_common.glow_color,
                                        coef),
                                    coef);
                                bg_color = TCOD_color_lerp(
                                    bg_color,
                                    TCOD_color_multiply_scalar(
                                        fg_color,
                                        actor_common.glow_intensity),
                                    coef);
                            }
                        }

                        TCOD_LIST_FOREACH(map->objects)
                        {
                            struct object *object = *iterator;
                            if (object->light_fov && TCOD_map_is_in_fov(object->light_fov, x, y))
                            {
                                float radius_sq = powf((float)object->light_radius, 2);
                                float distance_sq =
                                    powf((float)(x - object->x + (object->light_flicker ? dx : 0)), 2) +
                                    powf((float)(y - object->y + (object->light_flicker ? dy : 0)), 2);
                                float coef = CLAMP(
                                    0.0f,
                                    1.0f,
                                    (radius_sq - distance_sq) / radius_sq + (object->light_flicker ? di : 0));
                                fg_color = TCOD_color_lerp(
                                    fg_color,
                                    TCOD_color_lerp(
                                        tile_data.color,
                                        object->light_color,
                                        coef),
                                    coef);
                                bg_color = TCOD_color_lerp(
                                    bg_color,
                                    TCOD_color_multiply_scalar(
                                        fg_color,
                                        object->light_intensity),
                                    coef);
                            }
                        }

                        TCOD_LIST_FOREACH(map->actors)
                        {
                            struct actor *actor = *iterator;
                            if (actor->torch_fov && TCOD_map_is_in_fov(actor->torch_fov, x, y))
                            {
                                float radius_sq = powf((float)actor_common.torch_radius, 2);
                                float distance_sq =
                                    powf(x - actor->x + dx, 2) +
                                    powf(y - actor->y + dy, 2);
                                float coef = CLAMP(
                                    0.0f,
                                    1.0f,
                                    (radius_sq - distance_sq) / radius_sq + di);
                                fg_color = TCOD_color_lerp(
                                    fg_color,
                                    TCOD_color_lerp(
                                        tile_data.color,
                                        actor_common.torch_color,
                                        coef),
                                    coef);
                                bg_color = TCOD_color_lerp(
                                    bg_color,
                                    TCOD_color_multiply_scalar(
                                        fg_color,
                                        actor_common.torch_intensity),
                                    coef);
                            }
                        }
                    }

                    if (tile->seen)
                    {
                        TCOD_console_set_char_foreground(
                            console,
                            x - view_x,
                            y - view_y,
                            fg_color);
                        TCOD_console_set_char(
                            console,
                            x - view_x,
                            y - view_y,
                            tile_data.glyph);
                    }

                    TCOD_console_set_char_background(
                        console,
                        x - view_x,
                        y - view_y,
                        bg_color,
                        TCOD_BKGND_SET);
                }
            }
        }
    }
    TCOD_LIST_FOREACH(map->corpses)
    {
        struct actor *corpse = *iterator;
        if (TCOD_map_is_in_fov(world->player->fov, corpse->x, corpse->y))
        {
            TCOD_console_set_char_foreground(
                console,
                corpse->x - view_x,
                corpse->y - view_y,
                TCOD_dark_red);
            TCOD_console_set_char(
                console,
                corpse->x - view_x,
                corpse->y - view_y,
                '%');
        }
    }
    TCOD_LIST_FOREACH(map->objects)
    {
        struct object *object = *iterator;
        if (TCOD_map_is_in_fov(world->player->fov, object->x, object->y))
        {
            TCOD_console_set_char_foreground(
                console,
                object->x - view_x,
                object->y - view_y,
                object->color);
            TCOD_console_set_char(
                console,
                object->x - view_x,
                object->y - view_y,
                object_datum[object->type].glyph);
        }
    }
    TCOD_LIST_FOREACH(map->items)
    {
        struct item *item = *iterator;
        if (TCOD_map_is_in_fov(world->player->fov, item->x, item->y))
        {
            enum base_item base_item = item_datum[item->type].base_item;
            struct base_item_data base_item_data = base_item_datum[base_item];
            TCOD_console_set_char_foreground(
                console,
                item->x - view_x,
                item->y - view_y,
                base_item_data.color);
            TCOD_console_set_char(
                console,
                item->x - view_x,
                item->y - view_y,
                base_item_data.glyph);
        }
    }
    TCOD_LIST_FOREACH(map->projectiles)
    {
        struct projectile *projectile = *iterator;
        int x = (int)projectile->x;
        int y = (int)projectile->y;
        if (TCOD_map_is_in_fov(world->player->fov, x, y))
        {
            TCOD_console_set_char_foreground(
                console,
                x - view_x,
                y - view_y,
                TCOD_white);
            TCOD_console_set_char(
                console,
                x - view_x,
                y - view_y,
                projectile->glyph);
        }
    }
    TCOD_LIST_FOREACH(map->objects)
    {
        struct object *object = *iterator;
        if ((object->type == OBJECT_TYPE_STAIR_DOWN || object->type == OBJECT_TYPE_STAIR_UP) &&
            TCOD_map_is_in_fov(world->player->fov, object->x, object->y))
        {
            TCOD_console_set_char_foreground(
                console,
                object->x - view_x,
                object->y - view_y,
                object->color);
            TCOD_console_set_char(
                console,
                object->x - view_x,
                object->y - view_y,
                object_datum[object->type].glyph);
        }
    }
    TCOD_LIST_FOREACH(map->actors)
    {
        struct actor *actor = *iterator;
        if (!actor->dead && TCOD_map_is_in_fov(world->player->fov, actor->x, actor->y))
        {
            TCOD_color_t color = class_datum[actor->class].color;
            if (actor->flash_fade_coef > 0)
            {
                color = TCOD_color_lerp(color, actor->flash_color, actor->flash_fade_coef);
            }
            TCOD_console_set_char_foreground(
                console,
                actor->x - view_x,
                actor->y - view_y,
                color);
            TCOD_console_set_char(
                console,
                actor->x - view_x,
                actor->y - view_y,
                race_datum[actor->race].glyph);
        }
    }

    if (targeting != TARGETING_NONE)
    {
        TCOD_console_set_char_foreground(
            console,
            target_x - 1 - view_x,
            target_y - view_y,
            TCOD_red);
        TCOD_console_set_char(
            console,
            target_x - 1 - view_x,
            target_y - view_y,
            '[');
        TCOD_console_set_char_foreground(
            console,
            target_x + 1 - view_x,
            target_y - view_y,
            TCOD_red);
        TCOD_console_set_char(
            console,
            target_x + 1 - view_x,
            target_y - view_y,
            ']');

        if (map_is_inside(target_x, target_y))
        {
            struct tile *tile = &map->tiles[target_x][target_y];
            if (TCOD_map_is_in_fov(world->player->fov, target_x, target_y))
            {
                if (tile->actor)
                {
                    TCOD_console_printf_ex(
                        console,
                        view_width / 2,
                        view_height - 2,
                        TCOD_BKGND_NONE,
                        TCOD_CENTER,
                        "Lv.%d %s",
                        tile->actor->level,
                        tile->actor->name);

                    goto done;
                }
                struct actor *corpse = TCOD_list_peek(tile->corpses);
                if (corpse)
                {
                    TCOD_console_printf_ex(
                        console,
                        view_width / 2,
                        view_height - 2,
                        TCOD_BKGND_NONE,
                        TCOD_CENTER,
                        "%s (dead)",
                        corpse->name);

                    goto done;
                }
                struct item *item = TCOD_list_peek(tile->items);
                if (item)
                {
                    TCOD_console_printf_ex(
                        console,
                        view_width / 2,
                        view_height - 2,
                        TCOD_BKGND_NONE,
                        TCOD_CENTER,
                        item_datum[item->type].name);

                    goto done;
                }
                if (tile->object)
                {
                    TCOD_console_printf_ex(
                        console,
                        view_width / 2,
                        view_height - 2,
                        TCOD_BKGND_NONE,
                        TCOD_CENTER,
                        object_datum[tile->object->type].name);

                    goto done;
                }
                TCOD_console_printf_ex(
                    console,
                    view_width / 2,
                    view_height - 2,
                    TCOD_BKGND_NONE,
                    TCOD_CENTER,
                    tile_datum[tile->type].name);
            done:;
            }
            else
            {
                if (tile->seen)
                {
                    TCOD_console_printf_ex(
                        console,
                        view_width / 2,
                        view_height - 2,
                        TCOD_BKGND_NONE,
                        TCOD_CENTER,
                        "%s (known)",
                        tile_datum[tile->type].name);
                }
                else
                {
                    TCOD_console_printf_ex(
                        console,
                        view_width / 2,
                        view_height - 2,
                        TCOD_BKGND_NONE,
                        TCOD_CENTER,
                        "Unknown");
                }
            }
        }
    }

    if (message_log_rect.visible)
    {
        TCOD_console_set_default_background(message_log_rect.console, TCOD_black);
        TCOD_console_set_default_foreground(message_log_rect.console, TCOD_white);
        TCOD_console_clear(message_log_rect.console);

        int y = 1;
        TCOD_LIST_FOREACH(world->messages)
        {
            struct message *message = *iterator;
            TCOD_console_set_default_foreground(message_log_rect.console, message->color);
            TCOD_console_printf(
                message_log_rect.console,
                1,
                y,
                message->text);
            y++;
        }

        TCOD_console_set_default_foreground(message_log_rect.console, TCOD_white);
        TCOD_console_printf_frame(
            message_log_rect.console,
            0,
            0,
            message_log_rect.width,
            message_log_rect.height,
            false,
            TCOD_BKGND_SET,
            "Log");

        TCOD_console_blit(
            message_log_rect.console,
            0,
            0,
            message_log_rect.width,
            message_log_rect.height,
            console,
            message_log_rect.x,
            message_log_rect.y,
            1.0f,
            1.0f);
    }

    if (panel_rect.visible)
    {
        TCOD_console_set_default_background(panel_rect.console, TCOD_black);
        TCOD_console_set_default_foreground(panel_rect.console, TCOD_white);
        TCOD_console_clear(panel_rect.console);

        struct panel_state *current_panel_status = &panel_state[current_panel];
        switch (current_panel)
        {
        case PANEL_CHARACTER:
        {
            int y = 1;
            TCOD_console_printf(
                panel_rect.console,
                1,
                y++ - current_panel_status->scroll,
                "NAME     : %s",
                world->player->name);
            TCOD_console_printf(
                panel_rect.console,
                1,
                y++ - current_panel_status->scroll,
                "ALIGNMENT: Neutral Good");
            TCOD_console_printf(
                panel_rect.console,
                1,
                y++ - current_panel_status->scroll,
                "RACE     : %s",
                race_datum[world->player->race].name);
            TCOD_console_printf(
                panel_rect.console,
                1,
                y++ - current_panel_status->scroll,
                "CLASS    : %s",
                class_datum[world->player->class].name);
            TCOD_console_printf(
                panel_rect.console,
                1,
                y++ - current_panel_status->scroll,
                "LEVEL    : %d",
                world->player->level);
            TCOD_console_printf(
                panel_rect.console,
                1,
                y++ - current_panel_status->scroll,
                "EXP      : %d / %d",
                world->player->experience,
                actor_calc_experience_to_level(world->player->level + 1));
            y++;
            for (enum ability ability = 0; ability < NUM_ABILITIES; ability++)
            {
                TCOD_console_printf(
                    panel_rect.console,
                    1,
                    y++ - current_panel_status->scroll,
                    "%s: %d | %d",
                    ability_datum[ability].abbreviation,
                    world->player->ability_scores[ability],
                    actor_calc_ability_modifier(world->player, ability));
            }
            y++;
            for (enum equip_slot equip_slot = EQUIP_SLOT_ARMOR; equip_slot < NUM_EQUIP_SLOTS; equip_slot++)
            {
                TCOD_color_t color =
                    equip_slot == panel_character_get_selected()
                        ? TCOD_yellow
                        : TCOD_white;
                TCOD_console_set_default_foreground(panel_rect.console, color);
                struct equip_slot_data equip_slot_data = equip_slot_datum[equip_slot];
                if (world->player->equipment[equip_slot])
                {
                    struct item *equipment = world->player->equipment[equip_slot];
                    struct item_data item_data = item_datum[equipment->type];
                    if (current_panel_status->selection_mode)
                    {
                        TCOD_console_printf(
                            panel_rect.console,
                            1,
                            y++ - current_panel_status->scroll,
                            "%c) %s: %s", equip_slot + 'a' - 1,
                            equip_slot_data.label,
                            item_data.name);
                    }
                    else
                    {
                        TCOD_console_printf(
                            panel_rect.console,
                            1,
                            y++ - current_panel_status->scroll,
                            "%s: %s", equip_slot_data.label,
                            item_data.name);
                    }
                }
                else
                {
                    if (current_panel_status->selection_mode)
                    {
                        TCOD_console_printf(
                            panel_rect.console,
                            1,
                            y++ - current_panel_status->scroll,
                            "%c) %s: N/A", equip_slot + 'a' - 1,
                            equip_slot_data.label);
                    }
                    else
                    {
                        TCOD_console_printf(
                            panel_rect.console,
                            1,
                            y++ - current_panel_status->scroll,
                            "%s: N/A", equip_slot_data.label);
                    }
                }
                TCOD_console_set_default_foreground(panel_rect.console, TCOD_white);
            }
            y++;
            TCOD_console_printf(
                panel_rect.console,
                1,
                y++ - current_panel_status->scroll,
                "AC: %d",
                actor_calc_armor_class(world->player));
            TCOD_console_printf(
                panel_rect.console,
                1,
                y++ - current_panel_status->scroll,
                "HP: %d / %d",
                world->player->current_hp,
                actor_calc_max_hp(world->player));
            y++;
            int num_dice;
            int die_to_roll;
            int crit_threat;
            int crit_mult;
            actor_calc_weapon(
                world->player,
                &num_dice,
                &die_to_roll,
                &crit_threat,
                &crit_mult);
            TCOD_console_printf(
                panel_rect.console,
                1,
                y++ - current_panel_status->scroll,
                "WEAPON: %dd%d (%d-20x%d)",
                num_dice,
                die_to_roll,
                crit_threat,
                crit_mult);
            TCOD_console_printf(
                panel_rect.console,
                1,
                y++ - current_panel_status->scroll,
                "ATTACK: +%d",
                actor_calc_attack_bonus(world->player));
            TCOD_console_printf(
                panel_rect.console,
                1,
                y++ - current_panel_status->scroll,
                "DAMAGE: +%d",
                actor_calc_damage_bonus(world->player));
            y++;

            TCOD_console_set_default_foreground(panel_rect.console, TCOD_white);
            TCOD_console_printf_frame(
                panel_rect.console,
                0,
                0,
                panel_rect.width,
                panel_rect.height,
                false,
                TCOD_BKGND_SET,
                "Character");
        }
        break;
        case PANEL_EXAMINE:
        {
            TCOD_console_set_default_foreground(panel_rect.console, TCOD_white);
            TCOD_console_printf_frame(
                panel_rect.console,
                0,
                0,
                panel_rect.width,
                panel_rect.height,
                false,
                TCOD_BKGND_SET,
                "Examine");
        }
        break;
        case PANEL_INVENTORY:
        {
            int y = 1;
            TCOD_LIST_FOREACH(world->player->items)
            {
                struct item *item = *iterator;
                struct item_data item_data = item_datum[item->type];
                TCOD_color_t color =
                    item == panel_inventory_get_selected()
                        ? TCOD_yellow
                        : base_item_datum[item_data.base_item].color;
                TCOD_console_set_default_foreground(panel_rect.console, color);
                if (current_panel_status->selection_mode)
                {
                    TCOD_console_printf(
                        panel_rect.console,
                        1,
                        y - current_panel_status->scroll,
                        "%c) %s",
                        y - 1 + 'a' - current_panel_status->scroll,
                        item_data.name);
                }
                else
                {
                    TCOD_console_printf(
                        panel_rect.console,
                        1,
                        y - current_panel_status->scroll,
                        item_data.name);
                }
                TCOD_console_set_default_foreground(panel_rect.console, TCOD_white);
                y++;
            }
            TCOD_console_printf_frame(
                panel_rect.console,
                0,
                0,
                panel_rect.width,
                panel_rect.height,
                false,
                TCOD_BKGND_SET,
                "Inventory");
        }
        break;
        case PANEL_SPELLBOOK:
        {
            for (int y = 1; y <= 26; y++)
            {
                TCOD_console_printf(
                    panel_rect.console,
                    1,
                    y - current_panel_status->scroll,
                    "%d) spell",
                    y);
            }

            TCOD_console_set_default_foreground(panel_rect.console, TCOD_white);
            TCOD_console_printf_frame(
                panel_rect.console,
                0,
                0,
                panel_rect.width,
                panel_rect.height,
                false,
                TCOD_BKGND_SET,
                "Spellbook");
        }
        break;
        }

        TCOD_console_blit(
            panel_rect.console,
            0,
            0,
            panel_rect.width,
            panel_rect.height,
            console,
            panel_rect.x,
            panel_rect.y,
            1.0f,
            1.0f);
    }

    if (tooltip_rect.visible)
    {
        TCOD_console_set_default_background(tooltip_rect.console, TCOD_black);
        TCOD_console_set_default_foreground(tooltip_rect.console, TCOD_white);
        TCOD_console_clear(tooltip_rect.console);

        int y = 1;
        TCOD_LIST_FOREACH(tooltip_options)
        {
            struct tooltip_option *option = *iterator;
            TCOD_color_t color =
                option == tooltip_get_selected()
                    ? TCOD_yellow
                    : TCOD_white;
            TCOD_console_set_default_foreground(tooltip_rect.console, color);
            TCOD_console_printf(
                tooltip_rect.console,
                1,
                y,
                option->text);
            y++;
        }

        TCOD_console_set_default_foreground(tooltip_rect.console, TCOD_white);
        TCOD_console_printf_frame(
            tooltip_rect.console,
            0,
            0,
            tooltip_rect.width,
            tooltip_rect.height,
            false,
            TCOD_BKGND_SET,
            "");

        TCOD_console_blit(
            tooltip_rect.console,
            0,
            0,
            tooltip_rect.width,
            tooltip_rect.height,
            console,
            tooltip_rect.x,
            tooltip_rect.y,
            1.0f,
            1.0f);
    }

    if (automoving)
    {
        int x = automove_x;
        int y = automove_y;

        if (automove_actor)
        {
            x = automove_actor->x;
            y = automove_actor->y;
        }

        TCOD_console_set_char_background(console, x - view_x, y - view_y, TCOD_red, TCOD_BKGND_SET);
    }
}

static void quit(void)
{
    if (world->state != WORLD_STATE_LOSE)
    {
        world_save(SAVE_PATH);
    }
    world_quit();

    TCOD_LIST_FOREACH(tooltip_options)
    {
        struct tooltip_option *tooltip_option = *iterator;
        tooltip_option_delete(tooltip_option);
    }
    TCOD_console_delete(message_log_rect.console);
    TCOD_console_delete(panel_rect.console);
    TCOD_console_delete(tooltip_rect.console);
    TCOD_list_delete(tooltip_options);
    TCOD_noise_delete(noise);
}

struct scene game_scene =
    {&init,
     &handle_event,
     &update,
     &render,
     &quit};
