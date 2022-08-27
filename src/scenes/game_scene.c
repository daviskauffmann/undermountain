#include "game_scene.h"

#include "../config.h"
#include "../game/actor.h"
#include "../game/assets.h"
#include "../game/explosion.h"
#include "../game/item.h"
#include "../game/message.h"
#include "../game/projectile.h"
#include "../game/spell.h"
#include "../game/util.h"
#include "../game/world.h"
#include "../print.h"
#include "../scene.h"
#include "menu_scene.h"
#include <assert.h>
#include <float.h>
#include <libtcod.h>
#include <malloc.h>
#include <math.h>
#include <stdio.h>
#include <string.h>

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

/* Targeting */

enum targeting_action
{
    TARGETING_ACTION_NONE,
    TARGETING_ACTION_LOOK,
    TARGETING_ACTION_EXAMINE,
    TARGETING_ACTION_READ,
    TARGETING_ACTION_SHOOT,
    TARGETING_ACTION_SPELL
};

static enum targeting_action targeting_action;
static int target_x;
static int target_y;
static struct item *targeting_item;

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
           x >= rect.x && x < rect.x + rect.width &&
           y >= rect.y && y < rect.y + rect.height;
}

/* Viewport */

static struct rect view_rect;

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
    struct tooltip_option *tooltip_option = malloc(sizeof(*tooltip_option));
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

        if (!iterator)
        {
            break;
        }
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

static struct tooltip_option *tooltip_option_mouseover(void)
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

/* HUD */

static struct rect hud_rect;

/* Message log */

static struct rect message_log_rect;

/* Status */

static struct rect status_rect;

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

static enum ability panel_character_ability_mouseover(void)
{
    if (panel_rect.visible && current_panel == PANEL_CHARACTER && !tooltip_rect.visible)
    {
        int y = 9; // must be updated every time the layout of the character sheet changes

        for (enum ability ability = 0; ability < NUM_ABILITIES; ability++)
        {
            if (mouse_x > panel_rect.x &&
                mouse_x < panel_rect.x + panel_rect.width &&
                mouse_y == y + panel_rect.y - panel_state[current_panel].scroll)
            {
                return ability;
            }

            y++;
        }
    }

    return -1;
}

static enum equip_slot panel_character_equip_slot_mouseover(void)
{
    if (panel_rect.visible && current_panel == PANEL_CHARACTER && !tooltip_rect.visible)
    {
        int y = 14; // must be updated every time the layout of the character sheet changes

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

    return EQUIP_SLOT_NONE;
}

static struct item *panel_inventory_item_mouseover(void)
{
    if (panel_rect.visible && current_panel == PANEL_INVENTORY && !tooltip_rect.visible)
    {
        int y = 1;

        TCOD_LIST_FOREACH(world->player->items)
        {
            struct item *const item = *iterator;

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

static enum spell_type panel_spellbook_spell_type_mouseover(void)
{
    if (panel_rect.visible && current_panel == PANEL_SPELLBOOK && !tooltip_rect.visible)
    {
        int y = 1;

        TCOD_LIST_FOREACH(world->player->known_spell_types)
        {
            const enum spell_type spell_type = (size_t)(*iterator);

            if (mouse_x > panel_rect.x &&
                mouse_x < panel_rect.x + panel_rect.width &&
                mouse_y == y + panel_rect.y - panel_state[current_panel].scroll)
            {
                return spell_type;
            }

            y++;
        }
    }

    return SPELL_TYPE_NONE;
}

/* Cardianl directions */

enum direction
{
    DIRECTION_N,
    DIRECTION_NE,
    DIRECTION_E,
    DIRECTION_SE,
    DIRECTION_S,
    DIRECTION_SW,
    DIRECTION_W,
    DIRECTION_NW,
};

enum direction get_direction_from_angle(float angle)
{
    if ((angle > 11 * PI / 6 && angle <= 2 * PI) || (angle >= 0 && angle < PI / 6))
    {
        return DIRECTION_E;
    }
    else if (angle >= PI / 6 && angle <= PI / 3)
    {
        return DIRECTION_NE;
    }
    else if (angle > PI / 3 && angle < 2 * PI / 3)
    {
        return DIRECTION_N;
    }
    else if (angle >= 2 * PI / 3 && angle <= 5 * PI / 6)
    {
        return DIRECTION_NW;
    }
    else if (angle > 5 * PI / 6 && angle < 7 * PI / 6)
    {
        return DIRECTION_W;
    }
    else if (angle >= 7 * PI / 6 && angle <= 4 * PI / 3)
    {
        return DIRECTION_SW;
    }
    else if (angle > 4 * PI / 3 && angle < 5 * PI / 3)
    {
        return DIRECTION_S;
    }
    else if (angle >= 5 * PI / 3 && angle <= 11 * PI / 6)
    {
        return DIRECTION_SE;
    }
    assert(false);
    return -1;
}

void get_neighbor_by_direction(int x, int y, enum direction direction, int *nx, int *ny)
{
    *nx = x;
    *ny = y;
    switch (direction)
    {
    case DIRECTION_N:
        (*ny)--;
        break;
    case DIRECTION_NE:
        (*ny)--;
        (*nx)++;
        break;
    case DIRECTION_E:
        (*nx)++;
        break;
    case DIRECTION_SE:
        (*nx)++;
        (*ny)++;
        break;
    case DIRECTION_S:
        (*ny)++;
        break;
    case DIRECTION_SW:
        (*nx)--;
        (*ny)++;
        break;
    case DIRECTION_W:
        (*nx)--;
        break;
    case DIRECTION_NW:
        (*nx)--;
        (*ny)--;
        break;
    }
}

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

static bool do_directional_action(enum direction direction)
{
    bool took_turn = false;

    int x;
    int y;
    get_neighbor_by_direction(world->player->x, world->player->y, direction, &x, &y);

    switch (directional_action)
    {
    case DIRECTIONAL_ACTION_NONE:
    {
    }
    break;
    case DIRECTIONAL_ACTION_CLOSE_DOOR:
    {
        took_turn = actor_close_door(world->player, x, y);
    }
    break;
    case DIRECTIONAL_ACTION_DRINK:
    {
        took_turn = actor_drink(world->player, x, y);
    }
    break;
    case DIRECTIONAL_ACTION_OPEN_CHEST:
    {
        took_turn = actor_open_chest(world->player, x, y);
    }
    break;
    case DIRECTIONAL_ACTION_OPEN_DOOR:
    {
        took_turn = actor_open_door(world->player, x, y);
    }
    break;
    case DIRECTIONAL_ACTION_PRAY:
    {
        took_turn = actor_pray(world->player, x, y);
    }
    break;
    case DIRECTIONAL_ACTION_SIT:
    {
        took_turn = actor_sit(world->player, x, y);
    }
    break;
    }

    directional_action = DIRECTIONAL_ACTION_NONE;

    return took_turn;
}

/* Inventory menu actions */

enum inventory_action
{
    INVENTORY_ACTION_NONE,
    INVENTORY_ACTION_DROP,
    INVENTORY_ACTION_EQUIP,
    INVENTORY_ACTION_EXAMINE,
    INVENTORY_ACTION_QUAFF,
    INVENTORY_ACTION_READ,
};

static enum inventory_action inventory_action;

static bool do_inventory_action(struct item *const item)
{
    bool took_turn = false;

    switch (inventory_action)
    {
    case INVENTORY_ACTION_NONE:
    {
    }
    break;
    case INVENTORY_ACTION_DROP:
    {
        if (world_player_can_take_turn())
        {
            took_turn = actor_drop(world->player, item);
        }
    }
    break;
    case INVENTORY_ACTION_EQUIP:
    {
        if (world_player_can_take_turn())
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
    case INVENTORY_ACTION_QUAFF:
    {
        if (world_player_can_take_turn())
        {
            took_turn = actor_quaff(world->player, item);
        }
    }
    break;
    case INVENTORY_ACTION_READ:
    {
        bool needs_target = false;

        const struct item_datum *const item_datum = &item_data[item->type];
        if (item_datum->type == BASE_ITEM_TYPE_SCROLL)
        {
            const struct spell_datum *const spell_datum = &spell_data[item_datum->spell_type];
            if (spell_datum->range == SPELL_RANGE_TARGET)
            {
                needs_target = true;
            }
        }

        if (needs_target)
        {
            targeting_action = TARGETING_ACTION_READ;
            target_x = world->player->x;
            target_y = world->player->y;
            targeting_item = item;
        }
        else
        {
            if (world_player_can_take_turn())
            {
                took_turn = actor_read(world->player, item, world->player->x, world->player->y);
            }
        }
    }
    break;
    }

    inventory_action = INVENTORY_ACTION_NONE;

    return took_turn;
}

/* Character menu actions */

enum character_action
{
    CHARACTER_ACTION_NONE,
    CHARACTER_ACTION_ABILITY_ADD_POINT,
    CHARACTER_ACTION_EQUIPMENT_EXAMINE,
    CHARACTER_ACTION_EQUIPMENT_UNEQUIP
};

static enum character_action character_action;

static bool do_character_action_ability(const enum ability ability)
{
    bool took_turn = false;

    switch (character_action)
    {
    case CHARACTER_ACTION_NONE:
    {
    }
    break;
    case CHARACTER_ACTION_ABILITY_ADD_POINT:
    {
        if (world->player->ability_points > 0)
        {
            actor_add_ability_point(world->player, ability);
        }
    }
    break;
    case CHARACTER_ACTION_EQUIPMENT_EXAMINE:
    {
    }
    break;
    case CHARACTER_ACTION_EQUIPMENT_UNEQUIP:
    {
    }
    break;
    }

    return took_turn;
}

static bool do_character_action_equipment(const enum equip_slot equip_slot)
{
    bool took_turn = false;

    switch (character_action)
    {
    case CHARACTER_ACTION_NONE:
    {
    }
    break;
    case CHARACTER_ACTION_ABILITY_ADD_POINT:
    {
    }
    break;
    case CHARACTER_ACTION_EQUIPMENT_EXAMINE:
    {
        // TODO: send examine target to ui
        panel_show(PANEL_EXAMINE);
    }
    break;
    case CHARACTER_ACTION_EQUIPMENT_UNEQUIP:
    {
        if (world_player_can_take_turn())
        {
            took_turn = actor_unequip(world->player, equip_slot);
        }
    }
    break;
    }

    character_action = CHARACTER_ACTION_NONE;

    return took_turn;
}

/* Spellbook actions */

enum spellbook_action
{
    SPELLBOOK_ACTION_NONE,
    SPELLBOOK_ACTION_SELECT
};

static enum spellbook_action spellbook_action;

static bool do_spellbook_action(const enum spell_type spell_type)
{
    bool took_turn = false;

    switch (spellbook_action)
    {
    case SPELLBOOK_ACTION_NONE:
    {
    }
    break;
    case SPELLBOOK_ACTION_SELECT:
    {
        world->player->readied_spell_type = spell_type;

        world_log(
            world->player->floor,
            world->player->x,
            world->player->y,
            TCOD_yellow,
            "%s selected.",
            spell_data[spell_type].name);
    }
    break;
    }

    spellbook_action = SPELLBOOK_ACTION_NONE;

    return took_turn;
}

/* Noise */

static TCOD_noise_t noise;
static float noise_x;

static void init(struct scene *previous_scene)
{
    view_rect.console = NULL;
    view_rect.visible = true;

    tooltip_rect.console = TCOD_console_new(console_width, console_height);
    tooltip_options = TCOD_list_new();

    hud_rect.console = TCOD_console_new(console_width, console_height);
    hud_rect.visible = true;

    status_rect.console = TCOD_console_new(console_width, console_height);
    status_rect.visible = true;

    message_log_rect.console = TCOD_console_new(console_width, console_height);
    message_log_rect.visible = true;

    panel_rect.console = TCOD_console_new(console_width, console_height);
    current_panel = PANEL_CHARACTER;

    noise = TCOD_noise_new(1, TCOD_NOISE_DEFAULT_HURST, TCOD_NOISE_DEFAULT_LACUNARITY, NULL);
}

static void uninit(void)
{
    if (!world->hero_dead)
    {
        world_save(SAVE_PATH);
    }
    world_uninit();

    TCOD_noise_delete(noise);

    TCOD_console_delete(panel_rect.console);

    TCOD_console_delete(message_log_rect.console);

    TCOD_console_delete(status_rect.console);

    TCOD_console_delete(hud_rect.console);

    TCOD_LIST_FOREACH(tooltip_options)
    {
        tooltip_option_delete(*iterator);
    }
    TCOD_list_delete(tooltip_options);
    TCOD_console_delete(tooltip_rect.console);
}

static bool player_swing(enum direction direction)
{
    int x;
    int y;
    get_neighbor_by_direction(world->player->x, world->player->y, direction, &x, &y);

    if (map_is_inside(x, y))
    {
        struct item *weapon = world->player->equipment[EQUIP_SLOT_WEAPON];
        if (weapon)
        {
            const struct item_datum *const weapon_datum = &item_data[weapon->type];
            const struct base_item_datum *const base_weapon_datum = &base_item_data[weapon_datum->type];
            if (base_weapon_datum->ranged)
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

    return false;
}

static bool player_interact(SDL_Event *event, enum direction direction)
{
    if (directional_action != DIRECTIONAL_ACTION_NONE)
    {
        return do_directional_action(direction);
    }

    if (event->key.keysym.mod & KMOD_CTRL)
    {
        return player_swing(direction);
    }
    else
    {
        int x;
        int y;
        get_neighbor_by_direction(world->player->x, world->player->y, direction, &x, &y);
        return actor_move(world->player, x, y);
    }
}

static bool toolip_option_on_click_move(void)
{
    automoving = true;

    const struct tile *const tile = &world->maps[world->player->floor].tiles[tooltip_data.x][tooltip_data.y];

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

static struct scene *handle_event(SDL_Event *event)
{
    if (!world->player)
    {
        world->player = world->hero;
        if (!world->player)
        {
            return &game_scene;
        }
    }

    switch (event->type)
    {
    case SDL_KEYDOWN:
    {
        automoving = false;

        switch (event->key.keysym.sym)
        {
        case SDLK_ESCAPE:
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
            else if (targeting_action)
            {
                targeting_action = false;
            }
            else
            {
                game_scene.uninit();
                menu_scene.init(&game_scene);
                return &menu_scene;
            }
        }
        break;
        case SDLK_PAGEDOWN:
        {
            if (panel_rect.visible)
            {
                panel_state[current_panel].scroll++;
            }
        }
        break;
        case SDLK_PAGEUP:
        {
            if (panel_rect.visible)
            {
                panel_state[current_panel].scroll--;
            }
        }
        break;
        case SDLK_KP_1:
        {
            if (targeting_action != TARGETING_ACTION_NONE)
            {
                target_x--;
                target_y++;
            }
            else if (world_player_can_take_turn())
            {
                world->player->took_turn = player_interact(event, DIRECTION_SW);
            }
        }
        break;
        case SDLK_KP_2:
        case SDLK_DOWN:
        {
            if (targeting_action != TARGETING_ACTION_NONE)
            {
                target_y++;
            }
            else if (world_player_can_take_turn())
            {
                world->player->took_turn = player_interact(event, DIRECTION_S);
            }
        }
        break;
        case SDLK_KP_3:
        {
            if (targeting_action != TARGETING_ACTION_NONE)
            {
                target_x++;
                target_y++;
            }
            else if (world_player_can_take_turn())
            {
                world->player->took_turn = player_interact(event, DIRECTION_SE);
            }
        }
        break;
        case SDLK_KP_4:
        case SDLK_LEFT:
        {
            if (targeting_action != TARGETING_ACTION_NONE)
            {
                target_x--;
            }
            else if (world_player_can_take_turn())
            {
                world->player->took_turn = player_interact(event, DIRECTION_W);
            }
        }
        break;
        case SDLK_KP_5:
        {
            if (world_player_can_take_turn())
            {
                world->player->took_turn = true;
            }
        }
        break;
        case SDLK_KP_6:
        case SDLK_RIGHT:
        {
            if (targeting_action != TARGETING_ACTION_NONE)
            {
                target_x++;
            }
            else if (world_player_can_take_turn())
            {
                world->player->took_turn = player_interact(event, DIRECTION_E);
            }
        }
        break;
        case SDLK_KP_7:
        {
            if (targeting_action != TARGETING_ACTION_NONE)
            {
                target_x--;
                target_y--;
            }
            else if (world_player_can_take_turn())
            {
                world->player->took_turn = player_interact(event, DIRECTION_NW);
            }
        }
        break;
        case SDLK_KP_8:
        case SDLK_UP:
        {
            if (targeting_action != TARGETING_ACTION_NONE)
            {
                target_y--;
            }
            else if (world_player_can_take_turn())
            {
                world->player->took_turn = player_interact(event, DIRECTION_N);
            }
        }
        break;
        case SDLK_KP_9:
        {
            if (targeting_action != TARGETING_ACTION_NONE)
            {
                target_x++;
                target_y--;
            }
            else if (world_player_can_take_turn())
            {
                world->player->took_turn = player_interact(event, DIRECTION_NE);
            }
        }
        break;
        case SDLK_PERIOD:
        {
            if (event->key.keysym.mod & KMOD_SHIFT)
            {
                if (world_player_can_take_turn())
                {
                    world->player->took_turn = actor_descend(world->player, true, NULL);
                }
            }
        }
        break;
        case SDLK_COMMA:
        {
            if (event->key.keysym.mod & KMOD_SHIFT)
            {
                if (world_player_can_take_turn())
                {
                    world->player->took_turn = actor_ascend(world->player, true, NULL);
                }
            }
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
            bool handled = false;

            int alpha = event->key.keysym.sym - SDLK_a;
            if (inventory_action != INVENTORY_ACTION_NONE && alpha >= 0 && alpha < TCOD_list_size(world->player->items))
            {
                struct item *item = TCOD_list_get(world->player->items, alpha);

                world->player->took_turn = do_inventory_action(item);

                handled = true;
            }
            else if ((character_action == CHARACTER_ACTION_ABILITY_ADD_POINT) &&
                     alpha >= 0 && alpha < NUM_ABILITIES)
            {
                enum ability_type ability = (enum ability)alpha;

                world->player->took_turn = do_character_action_ability(ability);

                handled = true;
            }
            else if ((character_action == CHARACTER_ACTION_EQUIPMENT_EXAMINE ||
                      character_action == CHARACTER_ACTION_EQUIPMENT_UNEQUIP) &&
                     alpha >= 0 && alpha < NUM_EQUIP_SLOTS - 1)
            {
                enum equip_slot equip_slot = (enum equip_slot)(alpha + 1);

                world->player->took_turn = do_character_action_equipment(equip_slot);

                handled = true;
            }
            else if (spellbook_action != SPELLBOOK_ACTION_NONE && alpha >= 0 && alpha < TCOD_list_size(world->player->known_spell_types))
            {
                enum spell_type spell_type = (long long)TCOD_list_get(world->player->known_spell_types, alpha);

                world->player->took_turn = do_spellbook_action(spell_type);

                handled = true;
            }

            if (handled)
            {
                break;
            }

            switch (event->key.keysym.sym)
            {
            case SDLK_a:
            {
                panel_show(PANEL_CHARACTER);
                character_action = CHARACTER_ACTION_ABILITY_ADD_POINT;

                world_log(
                    world->player->floor,
                    world->player->x,
                    world->player->y,
                    TCOD_yellow,
                    "Choose abilities to add points to. Press 'ESC' to stop.");
            }
            break;
            case SDLK_b:
            {
                panel_toggle(PANEL_SPELLBOOK);
            }
            break;
            case SDLK_c:
            {
                if (event->key.keysym.mod & KMOD_SHIFT)
                {
                    panel_toggle(PANEL_CHARACTER);
                }
                else
                {
                    directional_action = DIRECTIONAL_ACTION_CLOSE_DOOR;

                    world_log(
                        world->player->floor,
                        world->player->x,
                        world->player->y,
                        TCOD_yellow,
                        "Choose a direction. Press 'ESC' to cancel.");
                }
            }
            break;
            case SDLK_d:
            {
                if (event->key.keysym.mod & KMOD_SHIFT)
                {
                    directional_action = DIRECTIONAL_ACTION_DRINK;

                    world_log(
                        world->player->floor,
                        world->player->x,
                        world->player->y,
                        TCOD_yellow,
                        "Choose a direction. Press 'ESC' to cancel.");
                }
                else
                {
                    panel_show(PANEL_INVENTORY);
                    inventory_action = INVENTORY_ACTION_DROP;

                    world_log(
                        world->player->floor,
                        world->player->x,
                        world->player->y,
                        TCOD_yellow,
                        "Choose an item to drop. Press 'ESC' to cancel.");
                }
            }
            break;
            case SDLK_e:
            {
                panel_show(PANEL_INVENTORY);
                inventory_action = INVENTORY_ACTION_EQUIP;

                world_log(
                    world->player->floor,
                    world->player->x,
                    world->player->y,
                    TCOD_yellow,
                    "Choose an item to equip. Press 'ESC' to cancel.");
            }
            break;
            case SDLK_f:
            {
                if (targeting_action == TARGETING_ACTION_SHOOT)
                {
                    if (world_player_can_take_turn())
                    {
                        actor_shoot(world->player, target_x, target_y);
                        targeting_action = TARGETING_ACTION_NONE;
                    }
                }
                else
                {
                    targeting_action = TARGETING_ACTION_SHOOT;

                    struct actor *target = actor_find_closest_enemy(world->player);
                    if (!target)
                    {
                        target = world->player;
                    }

                    target_x = target->x;
                    target_y = target->y;
                }
            }
            break;
            case SDLK_g:
            {
                if (world_player_can_take_turn())
                {
                    world->player->took_turn = actor_grab(world->player, world->player->x, world->player->y);
                }
            }
            break;
            case SDLK_h:
            {
                hud_rect.visible = !hud_rect.visible;
            }
            break;
            case SDLK_i:
            {
                panel_toggle(PANEL_INVENTORY);
            }
            break;
            case SDLK_k:
            {
                actor_die(world->player, NULL);
            }
            break;
            case SDLK_l:
            {
                if (targeting_action == TARGETING_ACTION_LOOK)
                {
                    targeting_action = TARGETING_ACTION_NONE;
                }
                else
                {
                    targeting_action = TARGETING_ACTION_LOOK;
                    target_x = world->player->x;
                    target_y = world->player->y;
                }
            }
            break;
            case SDLK_o:
            {
                if (event->key.keysym.mod & KMOD_SHIFT)
                {
                    directional_action = DIRECTIONAL_ACTION_OPEN_CHEST;

                    world_log(
                        world->player->floor,
                        world->player->x,
                        world->player->y,
                        TCOD_yellow,
                        "Choose a direction. Press 'ESC' to cancel.");
                }
                else
                {
                    directional_action = DIRECTIONAL_ACTION_OPEN_DOOR;

                    world_log(
                        world->player->floor,
                        world->player->x,
                        world->player->y,
                        TCOD_yellow,
                        "Choose a direction. Press 'ESC' to cancel.");
                }
            }
            break;
            case SDLK_p:
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
            case SDLK_q:
            {
                panel_show(PANEL_INVENTORY);
                inventory_action = INVENTORY_ACTION_QUAFF;

                world_log(
                    world->player->floor,
                    world->player->x,
                    world->player->y,
                    TCOD_yellow,
                    "Choose an item to quaff. Press 'ESC' to cancel.");
            }
            break;
            case SDLK_r:
            {
                if (targeting_action == TARGETING_ACTION_READ)
                {
                    if (world_player_can_take_turn())
                    {
                        world->player->took_turn = actor_read(
                            world->player,
                            targeting_item,
                            target_x, target_y);

                        targeting_action = TARGETING_ACTION_NONE;
                    }
                }
                else
                {
                    panel_show(PANEL_INVENTORY);
                    inventory_action = INVENTORY_ACTION_READ;

                    world_log(
                        world->player->floor,
                        world->player->x,
                        world->player->y,
                        TCOD_yellow,
                        "Choose an item to read. Press 'ESC' to cancel.");
                }
            }
            break;
            case SDLK_s:
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
            case SDLK_t:
            {
                if (event->key.keysym.mod & KMOD_SHIFT)
                {
                    if (world_player_can_take_turn())
                    {
                        if (world->player->light_type == LIGHT_TYPE_GLOW)
                        {
                            world->player->light_type = LIGHT_TYPE_NONE;
                        }
                        else
                        {
                            world->player->light_type = LIGHT_TYPE_GLOW;
                        }

                        world->player->took_turn = true;
                    }
                }
                else
                {
                    if (world_player_can_take_turn())
                    {
                        if (world->player->light_type == LIGHT_TYPE_TORCH)
                        {
                            world->player->light_type = LIGHT_TYPE_NONE;
                        }
                        else
                        {
                            world->player->light_type = LIGHT_TYPE_TORCH;
                        }

                        world->player->took_turn = true;
                    }
                }
            }
            break;
            case SDLK_u:
            {
                panel_show(PANEL_CHARACTER);
                character_action = CHARACTER_ACTION_EQUIPMENT_UNEQUIP;

                world_log(
                    world->player->floor,
                    world->player->x,
                    world->player->y,
                    TCOD_yellow,
                    "Choose an item to unequip. Press 'ESC' to cancel.");
            }
            break;
            case SDLK_x:
            {
                if (event->key.keysym.mod & KMOD_SHIFT)
                {
                    panel_show(PANEL_INVENTORY);
                    inventory_action = INVENTORY_ACTION_EXAMINE;

                    world_log(
                        world->player->floor,
                        world->player->x,
                        world->player->y,
                        TCOD_yellow,
                        "Choose an item to examine. Press 'ESC' to cancel.");
                }
                else
                {
                    if (event->key.keysym.mod & KMOD_CTRL)
                    {
                        panel_show(PANEL_CHARACTER);
                        character_action = CHARACTER_ACTION_EQUIPMENT_EXAMINE;

                        world_log(
                            world->player->floor,
                            world->player->x,
                            world->player->y,
                            TCOD_yellow,
                            "Choose an equipment to examine. Press 'ESC' to cancel.");
                    }
                    else
                    {
                        if (targeting_action == TARGETING_ACTION_EXAMINE)
                        {
                            panel_show(PANEL_EXAMINE); // TODO: send examine target to ui
                            targeting_action = TARGETING_ACTION_NONE;
                        }
                        else
                        {
                            targeting_action = TARGETING_ACTION_EXAMINE;
                            target_x = world->player->x;
                            target_y = world->player->y;
                        }
                    }
                }
            }
            break;
            case SDLK_z:
            {
                if (event->key.keysym.mod & KMOD_SHIFT)
                {
                    panel_show(PANEL_SPELLBOOK);
                    spellbook_action = SPELLBOOK_ACTION_SELECT;

                    world_log(
                        world->player->floor,
                        world->player->x,
                        world->player->y,
                        TCOD_yellow,
                        "Choose a spell. Press 'ESC' to cancel.");
                }
                else
                {
                    if (world->player->readied_spell_type != SPELL_TYPE_NONE)
                    {
                        enum spell_range spell_range = spell_data[world->player->readied_spell_type].range;
                        switch (spell_range)
                        {
                        case SPELL_RANGE_SELF:
                        {
                            if (world_player_can_take_turn())
                            {
                                world->player->took_turn = actor_cast_spell(
                                    world->player,
                                    world->player->readied_spell_type,
                                    world->player->x, world->player->y,
                                    true);
                            }
                        }
                        break;
                        case SPELL_RANGE_TARGET:
                        {
                            if (targeting_action == TARGETING_ACTION_SPELL)
                            {
                                if (world_player_can_take_turn())
                                {
                                    world->player->took_turn = actor_cast_spell(
                                        world->player,
                                        world->player->readied_spell_type,
                                        target_x, target_y,
                                        true);
                                    targeting_action = TARGETING_ACTION_NONE;
                                }
                            }
                            else
                            {
                                targeting_action = TARGETING_ACTION_SPELL;
                                target_x = world->player->x;
                                target_y = world->player->y;
                            }
                            break;
                        }
                        }
                    }
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
    case SDL_MOUSEBUTTONDOWN:
    {
        if (event->button.button == SDL_BUTTON_LEFT)
        {
            automoving = false;

            if (tooltip_rect.visible)
            {
                if (rect_is_inside(tooltip_rect, mouse_x, mouse_y))
                {
                    struct tooltip_option *tooltip_option = tooltip_option_mouseover();

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
            else if (rect_is_inside(panel_rect, mouse_x, mouse_y))
            {
                if (inventory_action != INVENTORY_ACTION_NONE)
                {
                    struct item *item = panel_inventory_item_mouseover();

                    if (item)
                    {
                        world->player->took_turn = do_inventory_action(item);
                    }
                }
                else if (character_action != CHARACTER_ACTION_NONE)
                {
                    if (character_action == CHARACTER_ACTION_ABILITY_ADD_POINT)
                    {
                        enum ability ability = panel_character_ability_mouseover();

                        if (ability != -1)
                        {
                            world->player->took_turn = do_character_action_ability(ability);
                        }
                    }
                    else if (character_action == CHARACTER_ACTION_EQUIPMENT_EXAMINE ||
                             character_action == CHARACTER_ACTION_EQUIPMENT_UNEQUIP)
                    {
                        enum equip_slot equip_slot = panel_character_equip_slot_mouseover();

                        if (equip_slot != EQUIP_SLOT_NONE)
                        {
                            world->player->took_turn = do_character_action_equipment(equip_slot);
                        }
                    }
                }
                else if (spellbook_action != SPELLBOOK_ACTION_NONE)
                {
                    enum spell_type spell_type = panel_spellbook_spell_type_mouseover();

                    if (spell_type != SPELL_TYPE_NONE)
                    {
                        world->player->took_turn = do_spellbook_action(spell_type);
                    }
                }
            }
            else if (map_is_inside(mouse_tile_x, mouse_tile_y))
            {
                if (world_player_can_take_turn())
                {
                    bool ranged = false;
                    struct item *weapon = world->player->equipment[EQUIP_SLOT_WEAPON];
                    if (weapon)
                    {
                        const struct item_datum *const item_datum = &item_data[weapon->type];
                        const struct base_item_datum *const base_item_datum = &base_item_data[item_datum->type];
                        if (base_item_datum->ranged)
                        {
                            ranged = true;
                        }
                    }

                    if (event->key.keysym.mod & KMOD_CTRL)
                    {
                        if (ranged)
                        {
                            actor_shoot(world->player, mouse_tile_x, mouse_tile_y);
                        }
                        else
                        {
                            float angle = angle_between(world->player->x, world->player->y, mouse_tile_x, mouse_tile_y);
                            enum direction direction = get_direction_from_angle(angle);
                            world->player->took_turn = player_swing(direction);
                        }
                    }
                    else
                    {
                        struct tile *tile = &world->maps[world->player->floor].tiles[mouse_tile_x][mouse_tile_y];
                        if (tile->actor && tile->actor->faction != world->player->faction)
                        {
                            if (ranged)
                            {
                                actor_shoot(world->player, tile->actor->x, tile->actor->y);
                            }
                            else
                            {
                                automoving = true;
                                automove_actor = tile->actor;
                            }
                        }
                        else
                        {
                            automoving = true;
                            automove_x = mouse_tile_x;
                            automove_y = mouse_tile_y;
                        }
                    }
                }
            }
        }
        else if (event->button.button == SDL_BUTTON_RIGHT)
        {
            if (rect_is_inside(panel_rect, mouse_x, mouse_y))
            {
                switch (current_panel)
                {
                case PANEL_CHARACTER:
                {
                    enum ability ability = panel_character_ability_mouseover();
                    if (ability != -1)
                    {
                        tooltip_show();
                        tooltip_options_add("Add Point", NULL);
                        // tooltip_data.ability = ability;
                        tooltip_options_add("Cancel", NULL);
                    }

                    enum equip_slot equip_slot = panel_character_equip_slot_mouseover();
                    if (equip_slot != EQUIP_SLOT_NONE)
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
                case PANEL_EXAMINE:
                {
                }
                break;
                case PANEL_INVENTORY:
                {
                    struct item *item = panel_inventory_item_mouseover();
                    if (item)
                    {
                        tooltip_show();
                        tooltip_options_add("Drop", NULL);
                        const struct item_datum *const item_datum = &item_data[item->type];
                        const struct base_item_datum *const base_item_datum = &base_item_data[item_datum->type];
                        if (base_item_datum->equip_slot != EQUIP_SLOT_NONE)
                        {
                            tooltip_options_add("Equip", &toolip_option_on_click_equip);
                        }
                        if (item_datum->type == BASE_ITEM_TYPE_POTION)
                        {
                            tooltip_options_add("Quaff", NULL);
                        }
                        tooltip_data.item = item;
                        tooltip_options_add("Cancel", NULL);
                    }
                }
                break;
                case PANEL_SPELLBOOK:
                {
                    enum spell_type spell_type = panel_spellbook_spell_type_mouseover();
                    if (spell_type != SPELL_TYPE_NONE)
                    {
                        tooltip_show();
                        if (world->player->readied_spell_type == spell_type)
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
                case NUM_PANELS:
                    break;
                }
            }
            else if (map_is_inside(mouse_tile_x, mouse_tile_y))
            {
                struct map *map = &world->maps[world->player->floor];
                struct tile *tile = &map->tiles[mouse_tile_x][mouse_tile_y];
                tooltip_show();
                tooltip_options_add("Move", &toolip_option_on_click_move);
                tooltip_options_add("Shoot", NULL); // TODO: only if ranged weapon equipped
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
                    if (tile->actor == world->player)
                    {
                        tooltip_options_add("Character Sheet", NULL);
                        tooltip_options_add("Inventory", NULL);
                        tooltip_options_add("Spellbook", NULL);
                    }
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
        }
    }
    break;
    case SDL_MOUSEWHEEL:
    {
        if (panel_rect.visible)
        {
            panel_state[current_panel].scroll -= event->wheel.y;
        }
    }
    break;
    case SDL_MOUSEMOTION:
    {
        mouse_x = event->motion.x;
        mouse_y = event->motion.y;
        mouse_tile_x = mouse_x + view_rect.x;
        mouse_tile_y = mouse_y + view_rect.y;
    }
    break;
    default:
        break;
    }

    return &game_scene;
}

static struct scene *update(TCOD_Console *const console, const float delta_time)
{
    if (!world->player)
    {
        world->player = world->hero;
        if (!world->player)
        {
            return &game_scene;
        }
    }

    if (automoving && world_player_can_take_turn())
    {
        if (automove_actor)
        {
            automove_x = automove_actor->x;
            automove_y = automove_actor->y;
        }

        // TODO: probably shouldnt use the path function for this
        // we need to implement custom behavior depending on what the player is doing
        // for example, if the player selects the interact option on a tooltip for an object far away,
        //      the player should navigate there but not interact/attack anything along the way
        world->player->took_turn = actor_path_towards(world->player, automove_x, automove_y);
        automoving = world->player->took_turn;

        // stop automoving if there is an enemy in FOV
        if (automoving)
        {
            const struct map *const map = &world->maps[world->player->floor];
            TCOD_LIST_FOREACH(map->actors)
            {
                const struct actor *const actor = *iterator;
                if (TCOD_map_is_in_fov(world->player->fov, actor->x, actor->y) &&
                    actor->faction != world->player->faction)
                {
                    automoving = false;
                }
            }
        }
    }
    else
    {
        automove_actor = NULL;
    }

    world_update(delta_time);

    // delete save if hero died
    if (world->hero_dead && TCOD_sys_file_exists(SAVE_PATH))
    {
        TCOD_sys_delete_file(SAVE_PATH);
    }

    // calculate ui sizes
    if (hud_rect.visible)
    {
        hud_rect.x = 0;
        hud_rect.width = console_width;
        hud_rect.height = console_height / 4;
        hud_rect.y = console_height - hud_rect.height;

        if (status_rect.visible)
        {
            // status should take up the bottom 4th and left 4th of the window
            status_rect.x = 0;
            status_rect.width = hud_rect.width / 4;
            status_rect.height = hud_rect.height;
            status_rect.y = hud_rect.height - status_rect.height;
        }

        if (message_log_rect.visible)
        {
            // message log should take up the bottom 4th and right 3/4ths of the window
            message_log_rect.x = hud_rect.width / 4;
            message_log_rect.width = hud_rect.width - message_log_rect.x;
            message_log_rect.height = hud_rect.height;
            message_log_rect.y = hud_rect.height - message_log_rect.height;
        }
    }

    if (panel_rect.visible)
    {
        // panel should take up the right half of the window
        panel_rect.y = 0;
        panel_rect.width = console_width / 2;
        panel_rect.x = console_width - panel_rect.width;
        panel_rect.height = console_height - (hud_rect.visible ? hud_rect.height : 0);
    }

    if (tooltip_rect.visible)
    {
        // tooltip should be as wide as the longest option, plus 2 for borders
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

        // tooltip should be as tall as the number of options, plus 2 for borders
        // TODO: multi-line options?
        tooltip_rect.height = TCOD_list_size(tooltip_options) + 2;

        // make sure the tooltip does not go off screen
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
    }

    if (view_rect.visible)
    {
        // determine size of the game area
        // it should be centered on the player, unless the player gets close to a map edge in which case it will stop
        view_rect.width = console_width - (panel_rect.visible ? panel_rect.width : 0);
        view_rect.height = console_height - (hud_rect.visible ? hud_rect.height : 0);
        view_rect.x = world->player ? world->player->x - view_rect.width / 2 : MAP_WIDTH / 2;
        view_rect.y = world->player ? world->player->y - view_rect.height / 2 : MAP_HEIGHT / 2;
        if (view_rect.x + view_rect.width > MAP_WIDTH)
        {
            view_rect.x = MAP_WIDTH - view_rect.width;
        }
        if (view_rect.x < 0)
        {
            view_rect.x = 0;
        }
        if (view_rect.y + view_rect.height > MAP_HEIGHT)
        {
            view_rect.y = MAP_HEIGHT - view_rect.height;
        }
        if (view_rect.y < 0)
        {
            view_rect.y = 0;
        }

        // calculate random noise coefficients
        noise_x += delta_time * 10;
        float noise_dx = noise_x + 20;
        float dx = TCOD_noise_get(noise, &noise_dx) * 0.5f;
        noise_dx += 30;
        float dy = TCOD_noise_get(noise, &noise_dx) * 0.5f;
        float di = 0.2f * TCOD_noise_get(noise, &noise_x);

        // get map to draw
        struct map *const map = &world->maps[world->player->floor];

        // draw tiles
        for (int x = view_rect.x; x < view_rect.x + view_rect.width; x++)
        {
            for (int y = view_rect.y; y < view_rect.y + view_rect.height; y++)
            {
                if (map_is_inside(x, y))
                {
                    struct tile *const tile = &map->tiles[x][y];
                    const struct tile_datum tile_datum = tile_data[tile->type];

                    // ambient lighting
                    float fg_r = tile_common.ambient_light_color.r;
                    float fg_g = tile_common.ambient_light_color.g;
                    float fg_b = tile_common.ambient_light_color.b;
                    float bg_r = fg_r * tile_common.ambient_light_intensity;
                    float bg_g = fg_g * tile_common.ambient_light_intensity;
                    float bg_b = fg_b * tile_common.ambient_light_intensity;

                    if (TCOD_map_is_in_fov(world->player->fov, x, y))
                    {
                        // mark this tile as seen
                        tile->seen = true;

                        // calculate object lighting
                        TCOD_LIST_FOREACH(map->objects)
                        {
                            struct object *object = *iterator;

                            if (object->light_fov && TCOD_map_is_in_fov(object->light_fov, x, y))
                            {
                                const float radius_sq = powf((float)object->light_radius, 2);
                                const float distance_sq =
                                    powf((float)(x - object->x + (object->light_flicker ? dx : 0)), 2) +
                                    powf((float)(y - object->y + (object->light_flicker ? dy : 0)), 2);
                                const float attenuation = CLAMP(
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

                        // calculate actor lighting
                        TCOD_LIST_FOREACH(map->actors)
                        {
                            struct actor *actor = *iterator;

                            if (actor->light_fov && TCOD_map_is_in_fov(actor->light_fov, x, y))
                            {
                                const struct light_datum *const light_datum = &light_data[actor->light_type];

                                const float radius_sq = powf((float)light_datum->radius, 2);
                                const float distance_sq =
                                    powf((float)(x - actor->x + (light_datum->flicker ? dx : 0)), 2) +
                                    powf((float)(y - actor->y + (light_datum->flicker ? dy : 0)), 2);
                                const float attenuation = CLAMP(
                                    0.0f,
                                    1.0f,
                                    (radius_sq - distance_sq) / radius_sq + (light_datum->flicker ? di : 0));

                                fg_r += light_datum->color.r * attenuation;
                                fg_g += light_datum->color.g * attenuation;
                                fg_b += light_datum->color.b * attenuation;
                                bg_r += light_datum->color.r * light_datum->intensity * attenuation;
                                bg_g += light_datum->color.g * light_datum->intensity * attenuation;
                                bg_b += light_datum->color.b * light_datum->intensity * attenuation;
                            }
                        }

                        // calculate projectile lighting
                        TCOD_LIST_FOREACH(map->projectiles)
                        {
                            struct projectile *projectile = *iterator;
                            const struct projectile_datum *const projectile_datum = &projectile_data[projectile->type];

                            if (projectile->light_fov && TCOD_map_is_in_fov(projectile->light_fov, x, y))
                            {
                                const struct light_datum *const light_datum = &light_data[projectile_datum->light_type];

                                const float radius_sq = powf((float)light_datum->radius, 2);
                                const float distance_sq =
                                    powf((float)(x - projectile->x + (light_datum->flicker ? dx : 0)), 2) +
                                    powf((float)(y - projectile->y + (light_datum->flicker ? dy : 0)), 2);
                                const float attenuation = CLAMP(
                                    0.0f,
                                    1.0f,
                                    (radius_sq - distance_sq) / radius_sq + (light_datum->flicker ? di : 0));

                                fg_r += light_datum->color.r * attenuation;
                                fg_g += light_datum->color.g * attenuation;
                                fg_b += light_datum->color.b * attenuation;
                                bg_r += light_datum->color.r * light_datum->intensity * attenuation;
                                bg_g += light_datum->color.g * light_datum->intensity * attenuation;
                                bg_b += light_datum->color.b * light_datum->intensity * attenuation;
                            }
                        }

                        // calculate explosion lighting
                        TCOD_LIST_FOREACH(map->explosions)
                        {
                            struct explosion *explosion = *iterator;

                            if (explosion->fov && TCOD_map_is_in_fov(explosion->fov, x, y))
                            {
                                const float radius_sq = powf((float)explosion->radius, 2);
                                const float distance_sq =
                                    powf((float)(x - explosion->x + dx * 2), 2) +
                                    powf((float)(y - explosion->y + dy * 2), 2);
                                const float attenuation = CLAMP(
                                    0.0f,
                                    1.0f,
                                    (radius_sq - distance_sq) / radius_sq + di * 2);

                                fg_r += explosion->color.r * attenuation;
                                fg_g += explosion->color.g * attenuation;
                                fg_b += explosion->color.b * attenuation;
                                bg_r += explosion->color.r * 0.5f * attenuation;
                                bg_g += explosion->color.g * 0.5f * attenuation;
                                bg_b += explosion->color.b * 0.5f * attenuation;
                            }
                        }
                    }

                    // apply tonemapping
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

                        // select appropriate wall graphic
                        if (tile->type == TILE_TYPE_WALL)
                        {
                            const int glyphs[] = {
                                0x25A0, //  0 - none = ■
                                0x2502, //  1 - N    = │
                                0x2500, //  2 - E    = ─
                                0x2514, //  3 - NE   = └
                                0x2502, //  4 - S    = │
                                0x2502, //  5 - NS   = │
                                0x250C, //  6 - SE   = ┌
                                0x251C, //  7 - NES  = ├
                                0x2500, //  8 - W    = ─
                                0x2518, //  9 - NW   = ┘
                                0x2500, // 10 - EW   = ─
                                0x2534, // 11 - NEW  = ┴
                                0x2510, // 12 - SW   = ┐
                                0x2524, // 13 - NSW  = ┤
                                0x252C, // 14 - ESW  = ┬
                                0x253C  // 15 - NESW = ┼
                            };

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
                            x - view_rect.x,
                            y - view_rect.y,
                            fg_color);
                        TCOD_console_set_char(
                            console,
                            x - view_rect.x,
                            y - view_rect.y,
                            glyph);
                    }

                    TCOD_console_set_char_background(
                        console,
                        x - view_rect.x,
                        y - view_rect.y,
                        bg_color,
                        TCOD_BKGND_SET);
                }
            }
        }

        // draw corpses
        TCOD_LIST_FOREACH(map->corpses)
        {
            struct corpse *corpse = *iterator;
            if (TCOD_map_is_in_fov(world->player->fov, corpse->x, corpse->y))
            {
                TCOD_console_set_char_foreground(
                    console,
                    corpse->x - view_rect.x,
                    corpse->y - view_rect.y,
                    corpse_common.color);
                TCOD_console_set_char(
                    console,
                    corpse->x - view_rect.x,
                    corpse->y - view_rect.y,
                    corpse_common.glyph);
            }
        }

        // draw objects (except stairs, they are drawn later)
        TCOD_LIST_FOREACH(map->objects)
        {
            struct object *object = *iterator;
            if (object->type != OBJECT_TYPE_STAIR_DOWN && object->type != OBJECT_TYPE_STAIR_UP &&
                TCOD_map_is_in_fov(world->player->fov, object->x, object->y))
            {
                TCOD_console_set_char_foreground(
                    console,
                    object->x - view_rect.x,
                    object->y - view_rect.y,
                    object->color);
                TCOD_console_set_char(
                    console,
                    object->x - view_rect.x,
                    object->y - view_rect.y,
                    object_data[object->type].glyph);
            }
        }

        // draw items
        TCOD_LIST_FOREACH(map->items)
        {
            struct item *item = *iterator;
            if (TCOD_map_is_in_fov(world->player->fov, item->x, item->y))
            {
                const struct item_datum *const item_datum = &item_data[item->type];
                const struct base_item_datum *const base_item_datum = &base_item_data[item_datum->type];
                TCOD_console_set_char_foreground(
                    console,
                    item->x - view_rect.x,
                    item->y - view_rect.y,
                    item_datum->color);
                TCOD_console_set_char(
                    console,
                    item->x - view_rect.x,
                    item->y - view_rect.y,
                    base_item_datum->glyph);
            }
        }

        // draw projectiles
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
                    const unsigned char glyphs[] = {
                        '|',   // N
                        '/',   // NE
                        '-',   // E
                        '\\',  // SE
                        '|',   // S
                        '/',   // SW
                        '-',   // W
                        '\\'}; // NW
                    float angle = angle_between(projectile->origin_x, projectile->origin_y, projectile->target_x, projectile->target_y);
                    enum direction direction = get_direction_from_angle(angle);
                    glyph = glyphs[direction];
                }
                TCOD_console_set_char_foreground(
                    console,
                    x - view_rect.x,
                    y - view_rect.y,
                    projectile_data[projectile->type].color);
                TCOD_console_set_char(
                    console,
                    x - view_rect.x,
                    y - view_rect.y,
                    glyph);
            }
        }

        // draw stairs (to make sure they are drawn on top of other entities)
        TCOD_LIST_FOREACH(map->objects)
        {
            struct object *object = *iterator;
            if ((object->type == OBJECT_TYPE_STAIR_DOWN || object->type == OBJECT_TYPE_STAIR_UP) &&
                TCOD_map_is_in_fov(world->player->fov, object->x, object->y))
            {
                TCOD_console_set_char_foreground(
                    console,
                    object->x - view_rect.x,
                    object->y - view_rect.y,
                    object->color);
                TCOD_console_set_char(
                    console,
                    object->x - view_rect.x,
                    object->y - view_rect.y,
                    object_data[object->type].glyph);
            }
        }

        // draw actors
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
                    actor->x - view_rect.x,
                    actor->y - view_rect.y,
                    color);
                TCOD_console_set_char(
                    console,
                    actor->x - view_rect.x,
                    actor->y - view_rect.y,
                    race_data[actor->race].glyph);
            }
        }

        // draw targeting reticle
        if (targeting_action != TARGETING_ACTION_NONE)
        {
            TCOD_console_set_char_foreground(
                console,
                target_x - 1 - view_rect.x,
                target_y - view_rect.y,
                TCOD_red);
            TCOD_console_set_char(
                console,
                target_x - 1 - view_rect.x,
                target_y - view_rect.y,
                '[');
            TCOD_console_set_char_foreground(
                console,
                target_x + 1 - view_rect.x,
                target_y - view_rect.y,
                TCOD_red);
            TCOD_console_set_char(
                console,
                target_x + 1 - view_rect.x,
                target_y - view_rect.y,
                ']');

            // descriptive text of target
            if (map_is_inside(target_x, target_y))
            {
                struct tile *tile = &map->tiles[target_x][target_y];
                if (TCOD_map_is_in_fov(world->player->fov, target_x, target_y))
                {
                    if (tile->actor)
                    {
                        TCOD_console_printf_ex(
                            console,
                            view_rect.width / 2,
                            view_rect.height - 2,
                            TCOD_BKGND_NONE,
                            TCOD_CENTER,
                            "Lv.%d %s",
                            tile->actor->level,
                            tile->actor->name);

                        goto done;
                    }
                    struct corpse *corpse = TCOD_list_peek(tile->corpses);
                    if (corpse)
                    {
                        TCOD_console_printf_ex(
                            console,
                            view_rect.width / 2,
                            view_rect.height - 2,
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
                        if (TCOD_list_size(tile->items) > 1)
                        {
                            TCOD_console_printf_ex(
                                console,
                                view_rect.width / 2,
                                view_rect.height - 2,
                                TCOD_BKGND_NONE,
                                TCOD_CENTER,
                                "%s (multiple)",
                                item_data[item->type].name);
                        }
                        else
                        {
                            TCOD_console_printf_ex(
                                console,
                                view_rect.width / 2,
                                view_rect.height - 2,
                                TCOD_BKGND_NONE,
                                TCOD_CENTER,
                                item_data[item->type].name);
                        }

                        goto done;
                    }
                    if (tile->object)
                    {
                        TCOD_console_printf_ex(
                            console,
                            view_rect.width / 2,
                            view_rect.height - 2,
                            TCOD_BKGND_NONE,
                            TCOD_CENTER,
                            object_data[tile->object->type].name);

                        goto done;
                    }
                    TCOD_console_printf_ex(
                        console,
                        view_rect.width / 2,
                        view_rect.height - 2,
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
                            view_rect.width / 2,
                            view_rect.height - 2,
                            TCOD_BKGND_NONE,
                            TCOD_CENTER,
                            "%s (known)",
                            tile_data[tile->type].name);
                    }
                    else
                    {
                        TCOD_console_printf_ex(
                            console,
                            view_rect.width / 2,
                            view_rect.height - 2,
                            TCOD_BKGND_NONE,
                            TCOD_CENTER,
                            "Unknown");
                    }
                }
            }
        }
    }

    if (hud_rect.visible)
    {
        TCOD_console_clear(hud_rect.console);

        if (status_rect.visible)
        {
            TCOD_console_clear(status_rect.console);

            int y = 1;

            {
                const int max_health = actor_calc_max_hit_points(world->player);
                const TCOD_color_t fg =
                    (float)world->player->hit_points / max_health > 0.5f
                        ? TCOD_color_lerp(TCOD_yellow, TCOD_green, world->player->hit_points / (max_health * 0.5f))
                        : TCOD_color_lerp(TCOD_red, TCOD_yellow, world->player->hit_points / (max_health * 0.5f));

                console_print(
                    status_rect.console,
                    1,
                    y++,
                    &fg,
                    NULL,
                    TCOD_BKGND_NONE,
                    TCOD_LEFT,
                    "HP: %d/%d",
                    world->player->hit_points,
                    max_health);
            }

            {
                const int max_mana = actor_calc_max_mana_points(world->player);
                const TCOD_color_t fg = TCOD_color_lerp(TCOD_gray, TCOD_azure, (float)world->player->mana_points / max_mana);

                console_print(
                    status_rect.console,
                    1,
                    y++,
                    &fg,
                    NULL,
                    TCOD_BKGND_NONE,
                    TCOD_LEFT,
                    "MP: %d/%d",
                    world->player->mana_points,
                    max_mana);
            }

            console_print(
                status_rect.console,
                1,
                y++,
                &TCOD_white,
                NULL,
                TCOD_BKGND_NONE,
                TCOD_LEFT,
                "Floor: %d",
                world->player->floor);

            console_print(
                status_rect.console,
                1,
                y++,
                &TCOD_white,
                NULL,
                TCOD_BKGND_NONE,
                TCOD_LEFT,
                "Time: %d",
                world->time);

            TCOD_console_printf_frame(
                status_rect.console,
                0, 0, status_rect.width, status_rect.height,
                false,
                TCOD_BKGND_NONE,
                "Status");

            TCOD_console_blit(
                status_rect.console,
                0, 0, status_rect.width, status_rect.height,
                hud_rect.console,
                status_rect.x, status_rect.y,
                1.0f, 1.0f);
        }

        if (message_log_rect.visible)
        {
            TCOD_console_clear(message_log_rect.console);

            int y = message_log_rect.height - 2;
            int message_index = TCOD_list_size(world->messages) - 1;
            while (message_index >= 0)
            {
                struct message *message = TCOD_list_get(world->messages, message_index--);
                if (!message)
                {
                    break;
                }

                int lines = TCOD_console_get_height_rect_fmt(
                    message_log_rect.console,
                    1,
                    y,
                    message_log_rect.width - 2,
                    message_log_rect.height - 2,
                    message->text);
                if (y - lines < 0)
                {
                    break;
                }

                const TCOD_color_t fg = TCOD_color_lerp(TCOD_gray, message->color, (float)y / (message_log_rect.height - 2));
                console_print(
                    message_log_rect.console,
                    1,
                    y - lines + 1,
                    &fg,
                    NULL,
                    TCOD_BKGND_NONE,
                    TCOD_LEFT,
                    message->text);

                y -= lines;
            }

            TCOD_console_printf_frame(
                message_log_rect.console,
                0, 0, message_log_rect.width, message_log_rect.height,
                false,
                TCOD_BKGND_NONE,
                "Messages");

            TCOD_console_blit(
                message_log_rect.console,
                0, 0, message_log_rect.width, message_log_rect.height,
                hud_rect.console,
                message_log_rect.x, message_log_rect.y,
                1.0f, 1.0f);
        }

        TCOD_console_blit(
            hud_rect.console,
            0, 0, hud_rect.width, hud_rect.height,
            console,
            hud_rect.x, hud_rect.y,
            1.0f, 1.0f);
    }

    if (panel_rect.visible)
    {
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
                actor_calc_experience_for_level(world->player->level + 1));
            y++;

            TCOD_console_printf(
                panel_rect.console,
                1,
                y - current_panel_status->scroll,
                "Gold");
            TCOD_console_printf_ex(
                panel_rect.console,
                panel_rect.width - 2,
                y - current_panel_status->scroll,
                TCOD_BKGND_NONE,
                TCOD_RIGHT,
                "%d",
                world->player->gold);
            y++;

            y++;

            TCOD_console_printf(
                panel_rect.console,
                1,
                y - current_panel_status->scroll,
                "Ability Points");
            TCOD_console_printf_ex(
                panel_rect.console,
                panel_rect.width - 2,
                y - current_panel_status->scroll,
                TCOD_BKGND_NONE,
                TCOD_RIGHT,
                "%d",
                world->player->ability_points);
            y++;

            for (enum ability ability = 0; ability < NUM_ABILITIES; ability++)
            {
                const TCOD_color_t fg =
                    ability == panel_character_ability_mouseover()
                        ? TCOD_yellow
                        : TCOD_white;

                if (character_action == CHARACTER_ACTION_ABILITY_ADD_POINT)
                {
                    console_print(
                        panel_rect.console,
                        1,
                        y - current_panel_status->scroll,
                        &fg,
                        NULL,
                        TCOD_BKGND_NONE,
                        TCOD_LEFT,
                        "%c) %s",
                        ability + SDLK_a,
                        ability_data[ability].name);
                }
                else
                {
                    console_print(
                        panel_rect.console,
                        1,
                        y - current_panel_status->scroll,
                        &fg,
                        NULL,
                        TCOD_BKGND_NONE,
                        TCOD_LEFT,
                        "%s",
                        ability_data[ability].name);
                }

                console_print(
                    panel_rect.console,
                    panel_rect.width - 2,
                    y - current_panel_status->scroll,
                    &fg,
                    NULL,
                    TCOD_BKGND_NONE,
                    TCOD_RIGHT,
                    "%d (%d)",
                    world->player->ability_scores[ability],
                    actor_calc_ability_modifer(world->player, ability));
                y++;
            }

            y++;

            for (enum equip_slot equip_slot = EQUIP_SLOT_NONE + 1; equip_slot < NUM_EQUIP_SLOTS; equip_slot++)
            {
                const TCOD_color_t fg =
                    equip_slot == panel_character_equip_slot_mouseover()
                        ? TCOD_yellow
                        : TCOD_white;
                const struct equip_slot_datum equip_slot_datum = equip_slot_data[equip_slot];

                if (character_action == CHARACTER_ACTION_EQUIPMENT_EXAMINE ||
                    character_action == CHARACTER_ACTION_EQUIPMENT_UNEQUIP)
                {
                    console_print(
                        panel_rect.console,
                        1,
                        y - current_panel_status->scroll,
                        &fg,
                        NULL,
                        TCOD_BKGND_NONE,
                        TCOD_LEFT,
                        "%c) %s",
                        equip_slot + SDLK_a - 1,
                        equip_slot_datum.name);
                }
                else
                {
                    console_print(
                        panel_rect.console,
                        1,
                        y - current_panel_status->scroll,
                        &fg,
                        NULL,
                        TCOD_BKGND_NONE,
                        TCOD_LEFT,
                        equip_slot_datum.name);
                }

                if (world->player->equipment[equip_slot])
                {
                    struct item *equipment = world->player->equipment[equip_slot];
                    struct item_datum item_datum = item_data[equipment->type];

                    console_print(
                        panel_rect.console,
                        panel_rect.width - 2,
                        y - current_panel_status->scroll,
                        &fg,
                        NULL,
                        TCOD_BKGND_NONE,
                        TCOD_RIGHT,
                        equipment->stack > 1 ? "%s (%d)" : "%s",
                        item_datum.name,
                        equipment->stack);
                }

                y++;
            }

            y++;

            TCOD_console_printf(
                panel_rect.console,
                1,
                y - current_panel_status->scroll,
                "Hit Points");
            TCOD_console_printf_ex(
                panel_rect.console,
                panel_rect.width - 2,
                y - current_panel_status->scroll,
                TCOD_BKGND_NONE,
                TCOD_RIGHT,
                "%d / %d",
                world->player->hit_points,
                actor_calc_max_hit_points(world->player));
            y++;

            TCOD_console_printf(
                panel_rect.console,
                1,
                y - current_panel_status->scroll,
                "Mana Points");
            TCOD_console_printf_ex(
                panel_rect.console,
                panel_rect.width - 2,
                y - current_panel_status->scroll,
                TCOD_BKGND_NONE,
                TCOD_RIGHT,
                "%d / %d",
                world->player->mana_points,
                actor_calc_max_mana_points(world->player));
            y++;

            TCOD_console_printf(
                panel_rect.console,
                1,
                y - current_panel_status->scroll,
                "Armor Class");
            TCOD_console_printf_ex(
                panel_rect.console,
                panel_rect.width - 2,
                y - current_panel_status->scroll,
                TCOD_BKGND_NONE,
                TCOD_RIGHT,
                "%d",
                actor_calc_armor_class(world->player));
            y++;

            TCOD_console_printf(
                panel_rect.console,
                1,
                y - current_panel_status->scroll,
                "Attack Bonus");
            TCOD_console_printf_ex(
                panel_rect.console,
                panel_rect.width - 2,
                y - current_panel_status->scroll,
                TCOD_BKGND_NONE,
                TCOD_RIGHT,
                "%d",
                actor_calc_attack_bonus(world->player));
            y++;

            TCOD_console_printf(
                panel_rect.console,
                1,
                y - current_panel_status->scroll,
                "Damage");
            const int threat_range = actor_calc_threat_range(world->player);
            if (threat_range == 20)
            {
                TCOD_console_printf_ex(
                    panel_rect.console,
                    panel_rect.width - 2,
                    y - current_panel_status->scroll,
                    TCOD_BKGND_NONE,
                    TCOD_RIGHT,
                    "%s + %d (x%d)",
                    actor_calc_damage(world->player),
                    actor_calc_damage_bonus(world->player),
                    actor_calc_critical_multiplier(world->player));
            }
            else
            {
                TCOD_console_printf_ex(
                    panel_rect.console,
                    panel_rect.width - 2,
                    y - current_panel_status->scroll,
                    TCOD_BKGND_NONE,
                    TCOD_RIGHT,
                    "%s + %d (%d-20/x%d)",
                    actor_calc_damage(world->player),
                    actor_calc_damage_bonus(world->player),
                    threat_range,
                    actor_calc_critical_multiplier(world->player));
            }
            y++;

            TCOD_console_printf(
                panel_rect.console,
                1,
                y - current_panel_status->scroll,
                "Speed");
            TCOD_console_printf_ex(
                panel_rect.console,
                panel_rect.width - 2,
                y - current_panel_status->scroll,
                TCOD_BKGND_NONE,
                TCOD_RIGHT,
                "%.1f",
                actor_calc_speed(world->player));
            y++;

            TCOD_console_printf_frame(
                panel_rect.console,
                0, 0, panel_rect.width, panel_rect.height,
                false,
                TCOD_BKGND_NONE,
                "Character");
        }
        break;
        case PANEL_EXAMINE:
        {
            TCOD_console_printf_frame(
                panel_rect.console,
                0, 0, panel_rect.width, panel_rect.height,
                false,
                TCOD_BKGND_NONE,
                "Examine");
        }
        break;
        case PANEL_INVENTORY:
        {
            struct item *mouseover_item = panel_inventory_item_mouseover();
            int y = 1;
            TCOD_LIST_FOREACH(world->player->items)
            {
                struct item *item = *iterator;
                struct item_datum item_datum = item_data[item->type];
                const TCOD_color_t fg =
                    item == mouseover_item
                        ? TCOD_yellow
                        : item_datum.color;

                if (inventory_action == INVENTORY_ACTION_NONE)
                {
                    console_print(
                        panel_rect.console,
                        1,
                        y - current_panel_status->scroll,
                        &fg,
                        NULL,
                        TCOD_BKGND_NONE,
                        TCOD_LEFT,
                        item->stack > 1 ? "%s (%d)" : "%s",
                        item_datum.name,
                        item->stack);
                }
                else
                {
                    console_print(
                        panel_rect.console,
                        1,
                        y - current_panel_status->scroll,
                        &fg,
                        NULL,
                        TCOD_BKGND_NONE,
                        TCOD_LEFT,
                        item->stack > 1 ? "%c) %s (%d)" : "%c) %s",
                        y - 1 + SDLK_a - current_panel_status->scroll,
                        item_datum.name,
                        item->stack);
                }

                y++;
            }

            TCOD_console_printf_frame(
                panel_rect.console,
                0, 0, panel_rect.width, panel_rect.height,
                false,
                TCOD_BKGND_SET,
                "Inventory");
        }
        break;
        case PANEL_SPELLBOOK:
        {
            enum spell_type mouseover_spell_type = panel_spellbook_spell_type_mouseover();
            int y = 1;
            TCOD_LIST_FOREACH(world->player->known_spell_types)
            {
                const enum spell_type spell_type = (size_t)(*iterator);
                const struct spell_datum *const spell_datum = &spell_data[spell_type];
                const TCOD_color_t fg =
                    spell_type == mouseover_spell_type
                        ? TCOD_yellow
                        : TCOD_white;

                if (spellbook_action == SPELLBOOK_ACTION_NONE)
                {
                    console_print(
                        panel_rect.console,
                        1,
                        y - current_panel_status->scroll,
                        &fg,
                        NULL,
                        TCOD_BKGND_NONE,
                        TCOD_LEFT,
                        world->player->readied_spell_type == spell_type ? "%s (readied)" : "%s",
                        spell_datum->name);
                }
                else
                {
                    console_print(
                        panel_rect.console,
                        1,
                        y - current_panel_status->scroll,
                        &fg,
                        NULL,
                        TCOD_BKGND_NONE,
                        TCOD_LEFT,
                        world->player->readied_spell_type == spell_type ? "%c) %s (readied)" : "%c) %s",
                        y - 1 + SDLK_a - current_panel_status->scroll,
                        spell_datum->name);
                }

                console_print(
                    panel_rect.console,
                    panel_rect.width - 2,
                    y - current_panel_status->scroll,
                    &fg,
                    NULL,
                    TCOD_BKGND_NONE,
                    TCOD_RIGHT,
                    "%d",
                    spell_datum->mana_cost);

                y++;
            }

            TCOD_console_printf_frame(
                panel_rect.console,
                0, 0, panel_rect.width, panel_rect.height,
                false,
                TCOD_BKGND_NONE,
                "Spellbook");
        }
        break;
        case NUM_PANELS:
            break;
        }

        TCOD_console_blit(
            panel_rect.console,
            0, 0, panel_rect.width, panel_rect.height,
            console,
            panel_rect.x,
            panel_rect.y,
            1.0f,
            1.0f);
    }

    if (tooltip_rect.visible)
    {
        TCOD_console_clear(tooltip_rect.console);

        const struct tooltip_option *const mouseover_tooltip_option = tooltip_option_mouseover();
        int y = 1;
        TCOD_LIST_FOREACH(tooltip_options)
        {
            const struct tooltip_option *const option = *iterator;
            const TCOD_color_t fg =
                option == mouseover_tooltip_option
                    ? TCOD_yellow
                    : TCOD_white;

            console_print(
                tooltip_rect.console,
                1,
                y,
                &fg,
                NULL,
                TCOD_BKGND_NONE,
                TCOD_LEFT,
                option->text);

            y++;
        }

        TCOD_console_printf_frame(
            tooltip_rect.console,
            0, 0, tooltip_rect.width, tooltip_rect.height,
            false,
            TCOD_BKGND_NONE,
            "");

        TCOD_console_blit(
            tooltip_rect.console,
            0, 0, tooltip_rect.width, tooltip_rect.height,
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

        TCOD_console_set_char_background(console, x - view_rect.x, y - view_rect.y, TCOD_red, TCOD_BKGND_SET);
    }

    if (!world->hero_dead && !world_player_can_take_turn())
    {
        console_print(
            console,
            0, 0,
            &TCOD_white,
            NULL,
            TCOD_BKGND_NONE,
            TCOD_LEFT,
            "Waiting...");
    }

    return &game_scene;
}

struct scene game_scene =
    {&init,
     &uninit,
     &handle_event,
     &update};
