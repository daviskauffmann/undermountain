#include "scene_game.h"

#include <assert.h>
#include <float.h>
#include <libtcod.h>
#include <malloc.h>
#include <math.h>
#include <stdio.h>
#include <string.h>

#include "../../game/actor.h"
#include "../../game/assets.h"
#include "../../game/item.h"
#include "../../game/message.h"
#include "../../game/projectile.h"
#include "../../game/spell.h"
#include "../../game/util.h"
#include "../../game/world.h"
#include "../config.h"
#include "../scene.h"
#include "../sys.h"
#include "scene_menu.h"

// TODO: better mouse controls
// design a system where when selecting an action from the right-click menu, other actions are ignored as the actor paths to the location
// when left-clicking a hostile actor, or selecting Attack from the menu, should track that actor's position and follow it. when reached, attack once and stop automoving

// TODO: automoving needs to be slower and easy to cancel

// TODO: prompts

/* Input variables */

static int mouse_x;
static int mouse_y;
static int mouse_tile_x;
static int mouse_tile_y;

static bool automoving;
static int automove_x;
static int automove_y;
static struct actor *automove_actor;

/* Directional actions */

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
    default:
        break;
    }

    directional_action = DIRECTIONAL_ACTION_NONE;

    return success;
}

/* Inventory menu actions */

enum inventory_action
{
    INVENTORY_ACTION_NONE,
    INVENTORY_ACTION_DROP,
    INVENTORY_ACTION_EQUIP,
    INVENTORY_ACTION_EXAMINE,
    INVENTORY_ACTION_QUAFF
};

static enum inventory_action inventory_action;

/* Character menu actions */

enum character_action
{
    CHARACTER_ACTION_NONE,
    CHARACTER_ACTION_EXAMINE,
    CHARACTER_ACTION_UNEQUIP
};

static enum character_action character_action;

/* Spellbook actions */

enum spellbook_action
{
    SPELLBOOK_ACTION_NONE,
    SPELLBOOK_ACTION_SELECT
};

static enum spellbook_action spellbook_action;

/* Targeting */

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

/* Viewport */

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

/* Noise */

static TCOD_noise_t noise;
static float noise_x;

/* Generic rect */

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

/* Tooltips */

struct tooltip_data
{
    int x;
    int y;
    struct object *object;
    struct item *item;
    struct actor *actor;
    enum equip_slot equip_slot;
    enum spell_type spell_type;
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
    tooltip_option->text = TCOD_strdup(text);
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
        iterator = TCOD_list_remove_iterator_fast(tooltip_options, iterator);
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

/* Message log */

static struct rect message_log_rect;

/* Side panel */

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
    if (panel_rect.visible && current_panel == PANEL_CHARACTER && !tooltip_rect.visible)
    {
        int y = 7;
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
    if (panel_rect.visible && current_panel == PANEL_INVENTORY && !tooltip_rect.visible)
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

static enum spell_type panel_spellbook_get_selected(void)
{
    if (panel_rect.visible && current_panel == PANEL_SPELLBOOK && !tooltip_rect.visible)
    {
        int y = 1;
        for (enum spell_type spell_type = 0; spell_type < NUM_SPELL_TYPES; spell_type++)
        {
            if (mouse_x > panel_rect.x &&
                mouse_x < panel_rect.x + panel_rect.width &&
                mouse_y == y + panel_rect.y - panel_state[current_panel].scroll)
            {
                return spell_type;
            }
            y++;
        }
    }
    return -1;
}

static void init(struct scene *previous_scene)
{
    tooltip_rect.console = TCOD_console_new(console_width, console_height);
    tooltip_options = TCOD_list_new();

    message_log_rect.console = TCOD_console_new(console_width, console_height);
    message_log_rect.visible = true;

    panel_rect.console = TCOD_console_new(console_width, console_height);
    current_panel = PANEL_CHARACTER;

    noise = TCOD_noise_new(1, TCOD_NOISE_DEFAULT_HURST, TCOD_NOISE_DEFAULT_LACUNARITY, NULL);
}

static bool player_interact(TCOD_key_t key, int x, int y)
{
    if (directional_action == DIRECTIONAL_ACTION_NONE)
    {
        if (key.lctrl)
        {
            if (map_is_inside(x, y))
            {
                struct item *weapon = world->player->equipment[EQUIP_SLOT_MAIN_HAND];
                if (weapon)
                {
                    if (item_data[weapon->type].ranged)
                    {
                        actor_shoot(world->player, x, y);
                        return false;
                    }
                }

                bool hit = false;
                struct map *map = &world->maps[world->player->floor];
                struct tile *tile = &map->tiles[x][y];
                if (tile->actor && tile->actor != world->player)
                {
                    hit = true;
                    if (actor_attack(world->player, tile->actor, NULL))
                    {
                        return true;
                    }
                }
                if (tile->object)
                {
                    hit = true;
                    if (actor_bash(world->player, tile->object))
                    {
                        return true;
                    }
                }
                if (!hit)
                {
                    world_log(
                        world->player->floor,
                        world->player->x,
                        world->player->y,
                        TCOD_white,
                        "%s swings at the air!",
                        world->player->name);
                }
            }
        }
        else
        {
            return actor_move(world->player, x, y);
        }
    }
    else
    {
        return do_directional_action(world->player, x, y);
    }

    return false;
}

static bool toolip_option_on_click_move(void)
{
    automoving = true;
    struct tile *tile = &world->maps[world->player->floor].tiles[tooltip_data.x][tooltip_data.y];
    if (tile->actor && tile->actor->faction != world->player->faction)
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

static bool toolip_option_on_click_equip(void)
{
    return actor_equip(world->player, tooltip_data.item);
}

static bool toolip_option_on_click_unequip(void)
{
    return actor_unequip(world->player, tooltip_data.equip_slot);
}

static struct scene *handle_event(TCOD_event_t ev, TCOD_key_t key, TCOD_mouse_t mouse)
{
    if (!world->player)
    {
        world->player = world->hero;
        if (!world->player)
        {
            return &game_scene;
        }
    }

    mouse_x = mouse.cx;
    mouse_y = mouse.cy;
    mouse_tile_x = mouse.cx + view_x;
    mouse_tile_y = mouse.cy + view_y;

    struct map *map = &world->maps[world->player->floor];
    bool can_take_turn =
        !world->hero_dead &&
        world->player == TCOD_list_get(map->actors, map->current_actor_index) &&
        TCOD_list_size(map->projectiles) == 0;

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
                     character_action != CHARACTER_ACTION_NONE ||
                     spellbook_action != SPELLBOOK_ACTION_NONE)
            {
                directional_action = DIRECTIONAL_ACTION_NONE;
                inventory_action = INVENTORY_ACTION_NONE;
                character_action = CHARACTER_ACTION_NONE;
                spellbook_action = SPELLBOOK_ACTION_NONE;
                for (enum panel panel = 0; panel < NUM_PANELS; panel++)
                {
                    panel_state[panel].selection_mode = false;
                }

                world_log(
                    world->player->floor,
                    world->player->x,
                    world->player->y,
                    TCOD_yellow,
                    "Action cancelled.");
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
            else if (can_take_turn)
            {
                int x = world->player->x - 1;
                int y = world->player->y + 1;
                world->player->took_turn = player_interact(key, x, y);
            }
        }
        break;
        case TCODK_KP2:
        case TCODK_DOWN:
        {
            if (targeting != TARGETING_NONE)
            {
                target_y++;
            }
            else if (can_take_turn)
            {
                int x = world->player->x;
                int y = world->player->y + 1;
                world->player->took_turn = player_interact(key, x, y);
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
            else if (can_take_turn)
            {
                int x = world->player->x + 1;
                int y = world->player->y + 1;
                world->player->took_turn = player_interact(key, x, y);
            }
        }
        break;
        case TCODK_KP4:
        case TCODK_LEFT:
        {
            if (targeting != TARGETING_NONE)
            {
                target_x--;
            }
            else if (can_take_turn)
            {
                int x = world->player->x - 1;
                int y = world->player->y;
                world->player->took_turn = player_interact(key, x, y);
            }
        }
        break;
        case TCODK_KP5:
        {
            if (can_take_turn)
            {
                world->player->took_turn = true;
            }
        }
        break;
        case TCODK_KP6:
        case TCODK_RIGHT:
        {
            if (targeting != TARGETING_NONE)
            {
                target_x++;
            }
            else if (can_take_turn)
            {
                int x = world->player->x + 1;
                int y = world->player->y;
                world->player->took_turn = player_interact(key, x, y);
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
            else if (can_take_turn)
            {
                int x = world->player->x - 1;
                int y = world->player->y - 1;
                world->player->took_turn = player_interact(key, x, y);
            }
        }
        break;
        case TCODK_KP8:
        case TCODK_UP:
        {
            if (targeting != TARGETING_NONE)
            {
                target_y--;
            }
            else if (can_take_turn)
            {
                int x = world->player->x;
                int y = world->player->y - 1;
                world->player->took_turn = player_interact(key, x, y);
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
            else if (can_take_turn)
            {
                int x = world->player->x + 1;
                int y = world->player->y - 1;
                world->player->took_turn = player_interact(key, x, y);
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
                    if (can_take_turn)
                    {
                        world->player->took_turn = actor_equip(world->player, item);
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
                    if (can_take_turn)
                    {
                        world->player->took_turn = actor_drop(world->player, item);
                    }
                }
                break;
                case INVENTORY_ACTION_QUAFF:
                {
                    if (can_take_turn)
                    {
                        world->player->took_turn = actor_quaff(world->player, item);
                    }
                }
                break;
                default:
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
                    if (can_take_turn)
                    {
                        world->player->took_turn = actor_unequip(world->player, equip_slot);
                    }
                }
                break;
                default:
                    break;
                }

                character_action = CHARACTER_ACTION_NONE;
                panel_state[PANEL_CHARACTER].selection_mode = false;

                handled = true;
            }
            else if (spellbook_action != SPELLBOOK_ACTION_NONE && alpha >= 0 && alpha < NUM_SPELL_TYPES)
            {
                enum spell_type spell_type = (enum spell_type)alpha;
                switch (spellbook_action)
                {
                case SPELLBOOK_ACTION_SELECT:
                {
                    world->player->readied_spell = spell_type;

                    world_log(
                        world->player->floor,
                        world->player->x,
                        world->player->y,
                        TCOD_yellow,
                        "%s selected.",
                        spell_data[world->player->readied_spell].name);
                }
                break;
                default:
                    break;
                }

                spellbook_action = SPELLBOOK_ACTION_NONE;
                panel_state[PANEL_SPELLBOOK].selection_mode = false;

                handled = true;
            }
            for (enum panel panel = 0; panel < NUM_PANELS; panel++)
            {
                if (panel_state[panel].selection_mode)
                {
                    handled = true;
                    break;
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
                if (can_take_turn)
                {
                    world->player->took_turn = actor_ascend(world->player, false, NULL);
                }
            }
            break;
            case '>':
            {
                if (can_take_turn)
                {
                    world->player->took_turn = actor_descend(world->player, false, NULL);
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
                directional_action = DIRECTIONAL_ACTION_CLOSE_DOOR;

                world_log(
                    world->player->floor,
                    world->player->x,
                    world->player->y,
                    TCOD_yellow,
                    "Choose a direction. Press 'ESC' to cancel.");
            }
            break;
            case 'D':
            {
                directional_action = DIRECTIONAL_ACTION_DRINK;

                world_log(
                    world->player->floor,
                    world->player->x,
                    world->player->y,
                    TCOD_yellow,
                    "Choose a direction. Press 'ESC' to cancel.");
            }
            break;
            case 'd':
            {
                panel_show(PANEL_INVENTORY);
                inventory_action = INVENTORY_ACTION_DROP;
                panel_state[PANEL_INVENTORY].selection_mode = true;

                world_log(
                    world->player->floor,
                    world->player->x,
                    world->player->y,
                    TCOD_yellow,
                    "Choose an item to drop. Press 'ESC' to cancel.");
            }
            break;
            case 'e':
            {
                panel_show(PANEL_INVENTORY);
                inventory_action = INVENTORY_ACTION_EQUIP;
                panel_state[PANEL_INVENTORY].selection_mode = true;

                world_log(
                    world->player->floor,
                    world->player->x,
                    world->player->y,
                    TCOD_yellow,
                    "Choose an item to equip. Press 'ESC' to cancel.");
            }
            break;
            case 'f':
            {
                if (targeting == TARGETING_SHOOT)
                {
                    if (can_take_turn)
                    {
                        actor_shoot(world->player, target_x, target_y);
                        targeting = TARGETING_NONE;
                    }
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
                            actor->faction != world->player->faction)
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
            break;
            case 'g':
            {
                if (can_take_turn)
                {
                    world->player->took_turn = actor_grab(world->player, world->player->x, world->player->y);
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
                directional_action = DIRECTIONAL_ACTION_OPEN_CHEST;

                world_log(
                    world->player->floor,
                    world->player->x,
                    world->player->y,
                    TCOD_yellow,
                    "Choose a direction. Press 'ESC' to cancel.");
            }
            break;
            case 'o':
            {
                directional_action = DIRECTIONAL_ACTION_OPEN_DOOR;

                world_log(
                    world->player->floor,
                    world->player->x,
                    world->player->y,
                    TCOD_yellow,
                    "Choose a direction. Press 'ESC' to cancel.");
            }
            break;
            case 'p':
            {
                directional_action = DIRECTIONAL_ACTION_PRAY;

                world_log(
                    world->player->floor,
                    world->player->x,
                    world->player->y,
                    TCOD_yellow,
                    "Choose a direction. Press 'ESC' to cancel.");
            }
            break;
            case 'q':
            {
                panel_show(PANEL_INVENTORY);

                inventory_action = INVENTORY_ACTION_QUAFF;
                panel_state[PANEL_INVENTORY].selection_mode = true;

                world_log(
                    world->player->floor,
                    world->player->x,
                    world->player->y,
                    TCOD_yellow,
                    "Choose an item to quaff. Press 'ESC' to cancel.");
            }
            break;
            case 's':
            {
                directional_action = DIRECTIONAL_ACTION_SIT;

                world_log(
                    world->player->floor,
                    world->player->x,
                    world->player->y,
                    TCOD_yellow,
                    "Choose a direction. Press 'ESC' to cancel.");
            }
            break;
            case 'T':
            {
                if (can_take_turn)
                {
                    if (world->player->light_radius >= 0 && TCOD_color_equals(world->player->light_color, actor_common.glow_color))
                    {
                        world->player->light_radius = -1;
                    }
                    else
                    {
                        world->player->light_radius = actor_common.glow_radius;
                        world->player->light_color = actor_common.glow_color;
                        world->player->light_intensity = actor_common.glow_intensity;
                        world->player->light_flicker = false;
                    }
                    world->player->took_turn = true;
                }
            }
            break;
            case 't':
            {
                if (can_take_turn)
                {
                    if (world->player->light_radius >= 0 && TCOD_color_equals(world->player->light_color, actor_common.torch_color))
                    {
                        world->player->light_radius = -1;
                    }
                    else
                    {
                        world->player->light_radius = actor_common.torch_radius;
                        world->player->light_color = actor_common.torch_color;
                        world->player->light_intensity = actor_common.torch_intensity;
                        world->player->light_flicker = true;
                    }
                    world->player->took_turn = true;
                }
            }
            break;
            case 'u':
            {
                panel_show(PANEL_CHARACTER);
                character_action = CHARACTER_ACTION_UNEQUIP;
                panel_state[PANEL_CHARACTER].selection_mode = true;

                world_log(
                    world->player->floor,
                    world->player->x,
                    world->player->y,
                    TCOD_yellow,
                    "Choose an item to unequip. Press 'ESC' to cancel.");
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
                    TCOD_yellow,
                    "Choose an item to examine. Press 'ESC' to cancel.");
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
                        TCOD_yellow,
                        "Choose an equipment to examine. Press 'ESC' to cancel.");
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
            case 'Z':
            {
                panel_show(PANEL_SPELLBOOK);
                spellbook_action = SPELLBOOK_ACTION_SELECT;
                panel_state[PANEL_SPELLBOOK].selection_mode = true;

                world_log(
                    world->player->floor,
                    world->player->x,
                    world->player->y,
                    TCOD_yellow,
                    "Choose a spell. Press 'ESC' to cancel.");
            }
            break;
            case 'z':
            {
                enum spell_range spell_range = spell_data[world->player->readied_spell].range;
                switch (spell_range)
                {
                case SPELL_RANGE_SELF:
                {
                    if (can_take_turn)
                    {
                        world->player->took_turn = actor_cast_spell(world->player, world->player->x, world->player->y);
                    }
                }
                break;
                case SPELL_RANGE_TARGET:
                {
                    if (targeting == TARGETING_SPELL)
                    {
                        if (can_take_turn)
                        {
                            world->player->took_turn = actor_cast_spell(world->player, target_x, target_y);
                            targeting = TARGETING_NONE;
                        }
                    }
                    else
                    {
                        targeting = TARGETING_SPELL;
                        target_x = world->player->x;
                        target_y = world->player->y;
                    }
                    break;
                }
                default:
                    break;
                }
            }
            break;
            default:
                break;
            }
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
                            world->player->took_turn = tooltip_option->on_click();
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
                if (tile->actor && tile->actor->faction != world->player->faction)
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
                            if (can_take_turn)
                            {
                                world->player->took_turn = actor_equip(world->player, item);
                            }
                        }
                        break;
                        case INVENTORY_ACTION_DROP:
                        {
                            if (can_take_turn)
                            {
                                world->player->took_turn = actor_drop(world->player, item);
                            }
                        }
                        break;
                        default:
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
                        switch (character_action)
                        {
                        case CHARACTER_ACTION_UNEQUIP:
                        {
                            if (can_take_turn)
                            {
                                world->player->took_turn = actor_unequip(world->player, equip_slot);
                            }
                        }
                        break;
                        default:
                            break;
                        }
                    }

                    character_action = CHARACTER_ACTION_NONE;
                    panel_state[PANEL_CHARACTER].selection_mode = false;
                }
                else if (spellbook_action != SPELLBOOK_ACTION_NONE)
                {
                    enum spell_type spell_type = panel_spellbook_get_selected();
                    if (spell_type >= 0 && spell_type < NUM_SPELL_TYPES)
                    {
                        switch (spellbook_action)
                        {
                        case SPELLBOOK_ACTION_SELECT:
                        {
                            world->player->readied_spell = spell_type;
                        }
                        break;
                        default:
                            break;
                        }
                    }

                    spellbook_action = SPELLBOOK_ACTION_NONE;
                    panel_state[PANEL_SPELLBOOK].selection_mode = false;
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
                        struct item_datum item_datum = item_data[item->type];
                        if (item_datum.equip_slot != EQUIP_SLOT_NONE)
                        {
                            tooltip_options_add("Equip", &toolip_option_on_click_equip);
                        }
                        if (item_datum.quaffable)
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
                            tooltip_options_add("Unequip", &toolip_option_on_click_unequip);
                            tooltip_data.equip_slot = equip_slot;
                            tooltip_options_add("Cancel", NULL);
                        }
                    }
                }
                break;
                case PANEL_SPELLBOOK:
                {
                    enum spell_type spell_type = panel_spellbook_get_selected();
                    if (spell_type >= 0 && spell_type < NUM_SPELL_TYPES)
                    {
                        tooltip_show();
                        if (world->player->readied_spell == spell_type)
                        {
                            tooltip_options_add("Unready", NULL);
                        }
                        else
                        {
                            tooltip_options_add("Ready", NULL);
                        }
                        tooltip_data.spell_type = spell_type;
                        tooltip_options_add("Cancel", NULL);
                    }
                }
                break;
                default:
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
    default:
        break;
    }

    if (automoving && can_take_turn)
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
        world->player->took_turn = actor_path_towards(world->player, automove_x, automove_y);
        automoving = world->player->took_turn;
    }
    else
    {
        automove_actor = NULL;
    }

    return &game_scene;
}

static struct scene *update(float delta_time)
{
    if (!world->player)
    {
        world->player = world->hero;
        if (!world->player)
        {
            return &game_scene;
        }
    }

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
    if (tooltip_rect.x + tooltip_rect.width > console_width)
    {
        tooltip_rect.x = console_width - tooltip_rect.width;
    }
    if (tooltip_rect.x < 0)
    {
        tooltip_rect.x = 0;
    }
    if (tooltip_rect.y + tooltip_rect.height > console_height)
    {
        tooltip_rect.y = console_height - tooltip_rect.height;
    }
    if (tooltip_rect.y < 0)
    {
        tooltip_rect.y = 0;
    }

    view_width = console_width - (panel_rect.visible ? panel_rect.width : 0);
    view_height = console_height - (message_log_rect.visible ? message_log_rect.height : 0);
    view_x = world->player ? world->player->x - view_width / 2 : MAP_WIDTH / 2;
    view_y = world->player ? world->player->y - view_height / 2 : MAP_HEIGHT / 2;
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

    world_update(delta_time);
    if (world->hero_dead && file_exists(SAVE_PATH))
    {
        remove(SAVE_PATH);
    }

    return &game_scene;
}

static void render(TCOD_console_t console)
{
    if (!world->player)
    {
        world->player = world->hero;
        if (!world->player)
        {
            return;
        }
    }

    TCOD_console_set_default_background(
        console,
        TCOD_color_multiply_scalar(
            tile_common.ambient_light_color,
            tile_common.ambient_light_intensity));
    TCOD_console_set_default_foreground(console, TCOD_white);

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
                    struct tile_datum tile_datum = tile_data[tile->type];
                    float fg_r = tile_common.ambient_light_color.r;
                    float fg_g = tile_common.ambient_light_color.g;
                    float fg_b = tile_common.ambient_light_color.b;
                    float bg_r = fg_r * tile_common.ambient_light_intensity;
                    float bg_g = fg_g * tile_common.ambient_light_intensity;
                    float bg_b = fg_b * tile_common.ambient_light_intensity;
                    if (TCOD_map_is_in_fov(world->player->fov, x, y))
                    {
                        tile->seen = true;

                        TCOD_LIST_FOREACH(map->objects)
                        {
                            struct object *object = *iterator;
                            if (object->light_fov && TCOD_map_is_in_fov(object->light_fov, x, y))
                            {
                                float radius_sq = powf((float)object->light_radius, 2);
                                float distance_sq =
                                    powf((float)(x - object->x + (object->light_flicker ? dx : 0)), 2) +
                                    powf((float)(y - object->y + (object->light_flicker ? dy : 0)), 2);
                                float attenuation = CLAMP(
                                    0.0f,
                                    1.0f,
                                    (radius_sq - distance_sq) / radius_sq + (object->light_flicker ? di : 0));
                                fg_r += object->light_color.r * attenuation;
                                fg_g += object->light_color.g * attenuation;
                                fg_b += object->light_color.b * attenuation;
                                bg_r += fg_r * object->light_intensity * attenuation;
                                bg_g += fg_g * object->light_intensity * attenuation;
                                bg_b += fg_b * object->light_intensity * attenuation;
                            }
                        }
                        TCOD_LIST_FOREACH(map->actors)
                        {
                            struct actor *actor = *iterator;
                            if (actor->light_fov && TCOD_map_is_in_fov(actor->light_fov, x, y))
                            {
                                float radius_sq = powf(actor->light_radius, 2);
                                float distance_sq =
                                    powf((float)(x - actor->x + (actor->light_flicker ? dx : 0)), 2) +
                                    powf((float)(y - actor->y + (actor->light_flicker ? dy : 0)), 2);
                                float attenuation = CLAMP(
                                    0.0f,
                                    1.0f,
                                    (radius_sq - distance_sq) / radius_sq + (actor->light_flicker ? di : 0));
                                fg_r += actor->light_color.r * attenuation;
                                fg_g += actor->light_color.g * attenuation;
                                fg_b += actor->light_color.b * attenuation;
                                bg_r += actor->light_color.r * actor->light_intensity * attenuation;
                                bg_g += actor->light_color.g * actor->light_intensity * attenuation;
                                bg_b += actor->light_color.b * actor->light_intensity * attenuation;
                            }
                        }
                    }
                    float fg_max = MAX(fg_r, MAX(fg_g, fg_b));
                    float fg_mult = fg_max > 255.0f ? 255.0f / fg_max : 1.0f;
                    TCOD_color_t fg_color = TCOD_color_RGB((uint8_t)(fg_r * fg_mult), (uint8_t)(fg_g * fg_mult), (uint8_t)(fg_b * fg_mult));
                    fg_color = TCOD_color_multiply(fg_color, tile_datum.color);
                    float bg_max = MAX(bg_r, MAX(bg_g, bg_b));
                    float bg_mult = bg_max > 255.0f ? 255.0f / bg_max : 1.0f;
                    TCOD_color_t bg_color = TCOD_color_RGB((uint8_t)(bg_r * bg_mult), (uint8_t)(bg_g * bg_mult), (uint8_t)(bg_b * bg_mult));
                    bg_color = TCOD_color_multiply(bg_color, tile_datum.color);
                    if (tile->seen)
                    {
                        int glyph = tile_datum.glyph;
                        if (tile->type == TILE_TYPE_WALL)
                        {
                            // select wall graphic
                            const int glyphs[] = {
                                TCOD_CHAR_BLOCK3,  //  0 - none
                                TCOD_CHAR_DVLINE,  //  1 - N
                                TCOD_CHAR_DHLINE,  //  2 - E
                                TCOD_CHAR_DSW,     //  3 - NE
                                TCOD_CHAR_DVLINE,  //  4 - S
                                TCOD_CHAR_DVLINE,  //  5 - NS
                                TCOD_CHAR_DNW,     //  6 - SE
                                TCOD_CHAR_DTEEE,   //  7 - NES
                                TCOD_CHAR_DHLINE,  //  8 - W
                                TCOD_CHAR_DSE,     //  9 - NW
                                TCOD_CHAR_DHLINE,  // 10 - EW
                                TCOD_CHAR_DTEEN,   // 11 - NEW
                                TCOD_CHAR_DNE,     // 12 - SW
                                TCOD_CHAR_DTEEW,   // 13 - NSW
                                TCOD_CHAR_DTEES,   // 14 - ESW
                                TCOD_CHAR_DCROSS}; // 15 - NESW
                            int index = 0;
                            if (y > 0 && map->tiles[x][y - 1].type == TILE_TYPE_WALL)
                            {
                                index |= 1 << 0;
                            }
                            if (x < MAP_WIDTH - 1 && map->tiles[x + 1][y].type == TILE_TYPE_WALL)
                            {
                                index |= 1 << 1;
                            }
                            if (y < MAP_HEIGHT - 1 && map->tiles[x][y + 1].type == TILE_TYPE_WALL)
                            {
                                index |= 1 << 2;
                            }
                            if (x > 0 && map->tiles[x - 1][y].type == TILE_TYPE_WALL)
                            {
                                index |= 1 << 3;
                            }
                            glyph = glyphs[index];
                        }
                        TCOD_console_set_char_foreground(
                            console,
                            x - view_x,
                            y - view_y,
                            fg_color);
                        TCOD_console_set_char(
                            console,
                            x - view_x,
                            y - view_y,
                            glyph);
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
        struct corpse *corpse = *iterator;
        if (TCOD_map_is_in_fov(world->player->fov, corpse->x, corpse->y))
        {
            TCOD_console_set_char_foreground(
                console,
                corpse->x - view_x,
                corpse->y - view_y,
                corpse_common.corpse_color);
            TCOD_console_set_char(
                console,
                corpse->x - view_x,
                corpse->y - view_y,
                corpse_common.corpse_glyph);
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
                object_data[object->type].glyph);
        }
    }
    TCOD_LIST_FOREACH(map->items)
    {
        struct item *item = *iterator;
        if (TCOD_map_is_in_fov(world->player->fov, item->x, item->y))
        {
            struct item_datum item_datum = item_data[item->type];
            TCOD_console_set_char_foreground(
                console,
                item->x - view_x,
                item->y - view_y,
                item_datum.color);
            TCOD_console_set_char(
                console,
                item->x - view_x,
                item->y - view_y,
                item_datum.glyph);
        }
    }
    TCOD_LIST_FOREACH(map->projectiles)
    {
        struct projectile *projectile = *iterator;
        int x = (int)projectile->x;
        int y = (int)projectile->y;
        if (TCOD_map_is_in_fov(world->player->fov, x, y))
        {
            char glyph = projectile_data[projectile->type].glyph;
            if (projectile->type == PROJECTILE_TYPE_ARROW)
            {
                float angle = projectile->angle;
                if ((angle > 11 * PI / 6 && angle <= 2 * PI) || (angle >= 0 && angle < PI / 6))
                {
                    glyph = '-';
                }
                else if (angle >= PI / 6 && angle <= PI / 3)
                {
                    glyph = '/';
                }
                else if (angle > PI / 3 && angle < 2 * PI / 3)
                {
                    glyph = '|';
                }
                else if (angle >= 2 * PI / 3 && angle <= 5 * PI / 6)
                {
                    glyph = '\\';
                }
                else if (angle > 5 * PI / 6 && angle < 7 * PI / 6)
                {
                    glyph = '-';
                }
                else if (angle >= 7 * PI / 6 && angle <= 4 * PI / 3)
                {
                    glyph = '/';
                }
                else if (angle > 4 * PI / 3 && angle < 5 * PI / 3)
                {
                    glyph = '|';
                }
                else if (angle >= 5 * PI / 3 && angle <= 11 * PI / 6)
                {
                    glyph = '\\';
                }
                else
                {
                    glyph = '`';
                }
            }
            TCOD_console_set_char_foreground(
                console,
                x - view_x,
                y - view_y,
                projectile_data[projectile->type].color);
            TCOD_console_set_char(
                console,
                x - view_x,
                y - view_y,
                glyph);
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
                object_data[object->type].glyph);
        }
    }
    TCOD_LIST_FOREACH(map->actors)
    {
        struct actor *actor = *iterator;
        if (TCOD_map_is_in_fov(world->player->fov, actor->x, actor->y))
        {
            TCOD_color_t color = class_data[actor->class].color;
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
                race_data[actor->race].glyph);
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
                        "Lv.%d %s (%s) (%f +%f)",
                        tile->actor->level,
                        tile->actor->name,
                        tile->actor->energy_per_turn < 0.75f ? "Slow" : tile->actor->energy_per_turn > 1.5f ? "Fast" : "Normal",
                        tile->actor->energy,
                        tile->actor->energy_per_turn);

                    goto done;
                }
                struct corpse *corpse = TCOD_list_peek(tile->corpses);
                if (corpse)
                {
                    TCOD_console_printf_ex(
                        console,
                        view_width / 2,
                        view_height - 2,
                        TCOD_BKGND_NONE,
                        TCOD_CENTER,
                        "Lv.%d %s (dead)",
                        corpse->level,
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
                        item_data[item->type].name);

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
                        object_data[tile->object->type].name);

                    goto done;
                }
                TCOD_console_printf_ex(
                    console,
                    view_width / 2,
                    view_height - 2,
                    TCOD_BKGND_NONE,
                    TCOD_CENTER,
                    tile_data[tile->type].name);
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
                        tile_data[tile->type].name);
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
                y - current_panel_status->scroll,
                "Name");
            TCOD_console_printf_ex(
                panel_rect.console,
                panel_rect.width - 2,
                y - current_panel_status->scroll,
                TCOD_BKGND_NONE,
                TCOD_RIGHT,
                world->player->name);
            y++;
            TCOD_console_printf(
                panel_rect.console,
                1,
                y - current_panel_status->scroll,
                "Race");
            TCOD_console_printf_ex(
                panel_rect.console,
                panel_rect.width - 2,
                y - current_panel_status->scroll,
                TCOD_BKGND_NONE,
                TCOD_RIGHT,
                race_data[world->player->race].name);
            y++;
            TCOD_console_printf(
                panel_rect.console,
                1,
                y - current_panel_status->scroll,
                "Class");
            TCOD_console_printf_ex(
                panel_rect.console,
                panel_rect.width - 2,
                y - current_panel_status->scroll,
                TCOD_BKGND_NONE,
                TCOD_RIGHT,
                class_data[world->player->class].name);
            y++;
            TCOD_console_printf(
                panel_rect.console,
                1,
                y - current_panel_status->scroll,
                "Level");
            TCOD_console_printf_ex(
                panel_rect.console,
                panel_rect.width - 2,
                y - current_panel_status->scroll,
                TCOD_BKGND_NONE,
                TCOD_RIGHT,
                "%d",
                world->player->level);
            y++;
            TCOD_console_printf(
                panel_rect.console,
                1,
                y - current_panel_status->scroll,
                "Experience");
            TCOD_console_printf_ex(
                panel_rect.console,
                panel_rect.width - 2,
                y - current_panel_status->scroll,
                TCOD_BKGND_NONE,
                TCOD_RIGHT,
                "%d / %d",
                world->player->experience,
                actor_calc_experience_to_level(world->player->level + 1));
            y++;
            y++;
            for (enum equip_slot equip_slot = EQUIP_SLOT_NONE + 1; equip_slot < NUM_EQUIP_SLOTS; equip_slot++)
            {
                TCOD_color_t color =
                    equip_slot == panel_character_get_selected()
                        ? TCOD_yellow
                        : TCOD_white;
                TCOD_console_set_default_foreground(panel_rect.console, color);
                struct equip_slot_datum equip_slot_datum = equip_slot_data[equip_slot];
                if (world->player->equipment[equip_slot])
                {
                    struct item *equipment = world->player->equipment[equip_slot];
                    struct item_datum item_datum = item_data[equipment->type];
                    if (current_panel_status->selection_mode)
                    {
                        TCOD_console_printf(
                            panel_rect.console,
                            1,
                            y - current_panel_status->scroll,
                            "%c) %s",
                            equip_slot + 'a' - 1,
                            equip_slot_datum.name);
                    }
                    else
                    {
                        TCOD_console_printf(
                            panel_rect.console,
                            1,
                            y - current_panel_status->scroll,
                            equip_slot_datum.name);
                    }
                    TCOD_console_printf_ex(
                        panel_rect.console,
                        panel_rect.width - 2,
                        y - current_panel_status->scroll,
                        TCOD_BKGND_NONE,
                        TCOD_RIGHT,
                        equipment->current_stack > 1 ? "%s (%d)" : "%s",
                        item_datum.name,
                        equipment->current_stack);
                    y++;
                }
                else
                {
                    if (current_panel_status->selection_mode)
                    {
                        TCOD_console_printf(
                            panel_rect.console,
                            1,
                            y - current_panel_status->scroll,
                            "%c) %s",
                            equip_slot + 'a' - 1,
                            equip_slot_datum.name);
                    }
                    else
                    {
                        TCOD_console_printf(
                            panel_rect.console,
                            1,
                            y - current_panel_status->scroll,
                            equip_slot_datum.name);
                    }
                    y++;
                }
                TCOD_console_set_default_foreground(panel_rect.console, TCOD_white);
            }
            y++;
            TCOD_console_printf(
                panel_rect.console,
                1,
                y - current_panel_status->scroll,
                "HP");
            TCOD_console_printf_ex(
                panel_rect.console,
                panel_rect.width - 2,
                y - current_panel_status->scroll,
                TCOD_BKGND_NONE,
                TCOD_RIGHT,
                "%d / %d",
                world->player->current_hp,
                world->player->max_hp);

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
                struct item_datum item_datum = item_data[item->type];
                TCOD_color_t color =
                    item == panel_inventory_get_selected()
                        ? TCOD_yellow
                        : item_datum.color;
                TCOD_console_set_default_foreground(panel_rect.console, color);
                if (current_panel_status->selection_mode)
                {
                    TCOD_console_printf(
                        panel_rect.console,
                        1,
                        y - current_panel_status->scroll,
                        item->current_stack > 1 ? "%c) %s (%d)" : "%c) %s",
                        y - 1 + 'a' - current_panel_status->scroll,
                        item_datum.name,
                        item->current_stack);
                }
                else
                {
                    TCOD_console_printf(
                        panel_rect.console,
                        1,
                        y - current_panel_status->scroll,
                        item->current_stack > 1 ? "%s (%d)" : "%s",
                        item_datum.name,
                        item->current_stack);
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
            int y = 1;
            for (enum spell_type spell_type = 0; spell_type < NUM_SPELL_TYPES; spell_type++)
            {
                struct spell_datum spell_datum = spell_data[spell_type];
                TCOD_color_t color =
                    spell_type == panel_spellbook_get_selected()
                        ? TCOD_yellow
                        : TCOD_white;
                TCOD_console_set_default_foreground(panel_rect.console, color);
                if (current_panel_status->selection_mode)
                {
                    TCOD_console_printf(
                        panel_rect.console,
                        1,
                        y - current_panel_status->scroll,
                        world->player->readied_spell == spell_type ? "%c) %s (readied)" : "%c) %s",
                        y - 1 + 'a' - current_panel_status->scroll,
                        spell_datum.name);
                }
                else
                {
                    TCOD_console_printf(
                        panel_rect.console,
                        1,
                        y - current_panel_status->scroll,
                        world->player->readied_spell == spell_type ? "%s (readied)" : "%s",
                        spell_datum.name);
                }
                TCOD_console_set_default_foreground(panel_rect.console, TCOD_white);
                y++;
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
        default:
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
            NULL);

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

    if (!world->hero_dead)
    {
        if (world->player != TCOD_list_get(map->actors, map->current_actor_index) ||
            TCOD_list_size(map->projectiles) > 0)
        {
            TCOD_console_printf(console, 0, 0, "%c", (char)31);
            TCOD_console_printf(console, 0, 1, "%c", (char)30);
        }
    }
}

static void quit(void)
{
    if (!world->hero_dead)
    {
        world_save(SAVE_PATH);
    }
    world_cleanup();

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
