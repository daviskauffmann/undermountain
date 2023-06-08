#include "game_scene.h"

#include "../../config.h"
#include "../../game/actor.h"
#include "../../game/assets.h"
#include "../../game/color.h"
#include "../../game/explosion.h"
#include "../../game/item.h"
#include "../../game/list.h"
#include "../../game/message.h"
#include "../../game/projectile.h"
#include "../../game/spell.h"
#include "../../game/util.h"
#include "../../game/world.h"
#include "../../print.h"
#include "../../scene.h"
#include "../../util.h"
#include "../menu/menu_scene.h"
#include "automove_action.h"
#include "character_action.h"
#include "direction.h"
#include "directional_action.h"
#include "inventory_action.h"
#include "panel.h"
#include "rect.h"
#include "spellbook_action.h"
#include "targeting_action.h"
#include "tooltip_data.h"
#include "tooltip_option.h"
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

static enum automove_action automove_action;
static int automove_x;
static int automove_y;
static struct actor *automove_actor;
static struct object *automove_object;

/* Targeting */

static enum targeting_action targeting_action;
static int target_x;
static int target_y;
static struct item *targeting_item;

/* Viewport */

static struct rect view_rect;

/* Tooltips */

static struct rect tooltip_rect;
static struct list *tooltip_options;
static struct tooltip_data tooltip_data;

static void add_tooltip_option(char *const text, bool (*const on_click)(void))
{
    struct tooltip_option *const tooltip_option = tooltip_option_new(text, on_click);

    list_add(tooltip_options, tooltip_option);
}

static void clear_tooltip_options(void)
{
    for (size_t tooltip_option_index = 0; tooltip_option_index < tooltip_options->size; tooltip_option_index++)
    {
        tooltip_option_delete(list_get(tooltip_options, tooltip_option_index));
    }

    list_clear(tooltip_options);
}

static void show_tooltip(void)
{
    clear_tooltip_options();

    tooltip_rect.visible = true;
    tooltip_rect.x = mouse_x;
    tooltip_rect.y = mouse_y;
}

static void hide_tooltip(void)
{
    clear_tooltip_options();

    tooltip_rect.visible = false;
}

static struct tooltip_option *tooltip_option_mouseover(void)
{
    if (tooltip_rect.visible)
    {
        int y = 1;

        for (size_t tooltip_option_index = 0; tooltip_option_index < tooltip_options->size; tooltip_option_index++)
        {
            struct tooltip_option *const option = list_get(tooltip_options, tooltip_option_index);

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

static struct rect panel_rect;
static struct panel_state panel_state[NUM_PANELS];
static enum panel current_panel;

static void toggle_panel(const enum panel panel)
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

static void show_panel(const enum panel panel)
{
    if (!panel_rect.visible || current_panel != panel)
    {
        toggle_panel(panel);
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

        for (size_t item_index = 0; item_index < world->player->items->size; item_index++)
        {
            struct item *const item = list_get(world->player->items, item_index);

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

        for (size_t spell_type_index = 0; spell_type_index < world->player->known_spell_types->size; spell_type_index++)
        {
            const enum spell_type spell_type = (size_t)(list_get(world->player->known_spell_types, spell_type_index));

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

/* Directional actions */

static enum directional_action directional_action;

static bool do_directional_action(enum direction direction)
{
    bool took_turn = false;

    int x;
    int y;
    get_neighbor_by_direction(world->player->x, world->player->y, direction, &x, &y);

    switch (directional_action)
    {
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

static enum inventory_action inventory_action;

static bool do_inventory_action(struct item *const item)
{
    bool took_turn = false;

    switch (inventory_action)
    {
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
        show_panel(PANEL_EXAMINE);
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

        const struct item_data *const item_data = &item_database[item->type];
        if (item_data->type == BASE_ITEM_TYPE_SCROLL)
        {
            const struct spell_data *const spell_data = &spell_database[item_data->spell_type];
            if (spell_data->range == SPELL_RANGE_TARGET)
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

static enum character_action character_action;

static bool do_character_action_ability(const enum ability ability)
{
    bool took_turn = false;

    switch (character_action)
    {
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

    // do not unset character action to allow consecutive point allocations

    return took_turn;
}

static bool do_character_action_equipment(const enum equip_slot equip_slot)
{
    bool took_turn = false;

    switch (character_action)
    {
    case CHARACTER_ACTION_ABILITY_ADD_POINT:
    {
    }
    break;
    case CHARACTER_ACTION_EQUIPMENT_EXAMINE:
    {
        // TODO: send examine target to ui
        show_panel(PANEL_EXAMINE);
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

static enum spellbook_action spellbook_action;

static bool do_spellbook_action(const enum spell_type spell_type)
{
    bool took_turn = false;

    switch (spellbook_action)
    {
    case SPELLBOOK_ACTION_SELECT:
    {
        world->player->readied_spell_type = spell_type;

        world_log(
            world->player->floor,
            world->player->x,
            world->player->y,
            color_yellow,
            "%s selected.",
            spell_database[spell_type].name);
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
    previous_scene;

    view_rect.console = NULL;
    view_rect.visible = true;

    tooltip_rect.console = TCOD_console_new(console_width, console_height);
    tooltip_options = list_new();

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
    if (!world->hero->dead)
    {
        world_save(SAVE_PATH);
    }
    world_uninit();

    TCOD_noise_delete(noise);

    TCOD_console_delete(panel_rect.console);

    TCOD_console_delete(message_log_rect.console);

    TCOD_console_delete(status_rect.console);

    TCOD_console_delete(hud_rect.console);

    for (size_t tooltip_option_index = 0; tooltip_option_index < tooltip_options->size; tooltip_option_index++)
    {
        tooltip_option_delete(list_get(tooltip_options, tooltip_option_index));
    }
    list_delete(tooltip_options);
    TCOD_console_delete(tooltip_rect.console);
}

static bool player_swing(enum direction direction)
{
    int x;
    int y;
    get_neighbor_by_direction(world->player->x, world->player->y, direction, &x, &y);

    if (map_is_inside(x, y))
    {
        if (actor_has_ranged_weapon(world->player))
        {
            if (actor_shoot(world->player, x, y))
            {
                return true;
            }
        }

        bool whiff = true;

        const struct map *const map = &world->maps[world->player->floor];
        const struct tile *const tile = &map->tiles[x][y];

        if (tile->actor && tile->actor != world->player)
        {
            whiff = false;

            if (actor_attack(world->player, tile->actor, NULL))
            {
                return true;
            }
        }

        if (tile->object)
        {
            whiff = false;

            if (actor_bash(world->player, tile->object))
            {
                return true;
            }
        }

        if (whiff)
        {
            world_log(
                world->player->floor,
                world->player->x,
                world->player->y,
                color_white,
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

static bool on_click_add_point(void)
{
    actor_add_ability_point(world->player, tooltip_data.ability);

    return false;
}

static bool on_click_move(void)
{
    automove_action = AUTOMOVE_ACTION_MOVE;

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

static bool on_click_shoot(void)
{
    return actor_shoot(world->player, tooltip_data.x, tooltip_data.y);
}

static bool on_click_examine(void)
{
    // TODO
    return false;
}

static bool on_click_interact(void)
{
    automove_action = AUTOMOVE_ACTION_INTERACT;
    automove_object = tooltip_data.object;

    return false;
}

static bool on_click_bash(void)
{
    automove_action = AUTOMOVE_ACTION_BASH;
    automove_object = tooltip_data.object;

    return false;
}

static bool on_click_swap(void)
{
    automove_action = AUTOMOVE_ACTION_SWAP;
    automove_actor = tooltip_data.actor;

    return false;
}

static bool on_click_attack(void)
{
    automove_action = AUTOMOVE_ACTION_ATTACK;
    automove_actor = tooltip_data.actor;

    return false;
}

static bool on_click_character_sheet(void)
{
    show_panel(PANEL_CHARACTER);

    return false;
}

static bool on_click_inventory(void)
{
    show_panel(PANEL_INVENTORY);

    return false;
}

static bool on_click_spellbook(void)
{
    show_panel(PANEL_SPELLBOOK);

    return false;
}

static bool on_click_drop(void)
{
    return actor_drop(world->player, tooltip_data.item);
}

static bool on_click_quaff(void)
{
    return actor_quaff(world->player, tooltip_data.item);
}

static bool on_click_equip(void)
{
    return actor_equip(world->player, tooltip_data.item);
}

static bool on_click_unequip(void)
{
    return actor_unequip(world->player, tooltip_data.equip_slot);
}

static bool on_click_unready(void)
{
    world->player->readied_spell_type = SPELL_TYPE_NONE;

    return false;
}

static bool on_click_ready(void)
{
    world->player->readied_spell_type = tooltip_data.spell_type;

    return false;
}

static bool on_click_cast(void)
{
    return actor_cast_spell(
        world->player,
        tooltip_data.spell_type,
        tooltip_data.x,
        tooltip_data.y,
        true);
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
        automove_action = AUTOMOVE_ACTION_NONE;

        switch (event->key.keysym.sym)
        {
        case SDLK_ESCAPE:
        {
            if (tooltip_rect.visible)
            {
                hide_tooltip();
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
                    color_yellow,
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
                world->player->took_turn = actor_rest(world->player);
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
                    world->player->took_turn = actor_descend(world->player);
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
                    world->player->took_turn = actor_ascend(world->player);
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

            const int alpha = event->key.keysym.sym - SDLK_a;
            if (inventory_action != INVENTORY_ACTION_NONE && alpha >= 0 && alpha < world->player->items->size)
            {
                struct item *const item = list_get(world->player->items, alpha);

                world->player->took_turn = do_inventory_action(item);

                handled = true;
            }
            else if ((character_action == CHARACTER_ACTION_ABILITY_ADD_POINT) &&
                     alpha >= 0 && alpha < NUM_ABILITIES)
            {
                const enum ability_type ability = alpha;

                world->player->took_turn = do_character_action_ability(ability);

                handled = true;
            }
            else if ((character_action == CHARACTER_ACTION_EQUIPMENT_EXAMINE ||
                      character_action == CHARACTER_ACTION_EQUIPMENT_UNEQUIP) &&
                     alpha >= 0 && alpha < NUM_EQUIP_SLOTS - 1)
            {
                const enum equip_slot equip_slot = alpha + 1;

                world->player->took_turn = do_character_action_equipment(equip_slot);

                handled = true;
            }
            else if (spellbook_action != SPELLBOOK_ACTION_NONE && alpha >= 0 && alpha < world->player->known_spell_types->size)
            {
                const enum spell_type spell_type = (size_t)list_get(world->player->known_spell_types, alpha);

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
                show_panel(PANEL_CHARACTER);
                character_action = CHARACTER_ACTION_ABILITY_ADD_POINT;

                world_log(
                    world->player->floor,
                    world->player->x,
                    world->player->y,
                    color_yellow,
                    "Choose abilities to add points to. Press 'ESC' to stop.");
            }
            break;
            case SDLK_b:
            {
                toggle_panel(PANEL_SPELLBOOK);
            }
            break;
            case SDLK_c:
            {
                if (event->key.keysym.mod & KMOD_SHIFT)
                {
                    toggle_panel(PANEL_CHARACTER);
                }
                else
                {
                    directional_action = DIRECTIONAL_ACTION_CLOSE_DOOR;

                    world_log(
                        world->player->floor,
                        world->player->x,
                        world->player->y,
                        color_yellow,
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
                        color_yellow,
                        "Choose a direction. Press 'ESC' to cancel.");
                }
                else
                {
                    show_panel(PANEL_INVENTORY);
                    inventory_action = INVENTORY_ACTION_DROP;

                    world_log(
                        world->player->floor,
                        world->player->x,
                        world->player->y,
                        color_yellow,
                        "Choose an item to drop. Press 'ESC' to cancel.");
                }
            }
            break;
            case SDLK_e:
            {
                show_panel(PANEL_INVENTORY);
                inventory_action = INVENTORY_ACTION_EQUIP;

                world_log(
                    world->player->floor,
                    world->player->x,
                    world->player->y,
                    color_yellow,
                    "Choose an item to equip. Press 'ESC' to cancel.");
            }
            break;
            case SDLK_f:
            {
                if (targeting_action == TARGETING_ACTION_SHOOT)
                {
                    if (world_player_can_take_turn())
                    {
                        world->player->took_turn = actor_shoot(world->player, target_x, target_y);
                        targeting_action = TARGETING_ACTION_NONE;
                    }
                }
                else
                {
                    targeting_action = TARGETING_ACTION_SHOOT;

                    const struct actor *nearest_enemy = actor_find_nearest_enemy(world->player);

                    if (!nearest_enemy)
                    {
                        nearest_enemy = world->player;
                    }

                    target_x = nearest_enemy->x;
                    target_y = nearest_enemy->y;
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
                toggle_panel(PANEL_INVENTORY);
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
                        color_yellow,
                        "Choose a direction. Press 'ESC' to cancel.");
                }
                else
                {
                    directional_action = DIRECTIONAL_ACTION_OPEN_DOOR;

                    world_log(
                        world->player->floor,
                        world->player->x,
                        world->player->y,
                        color_yellow,
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
                    color_yellow,
                    "Choose a direction. Press 'ESC' to cancel.");
            }
            break;
            case SDLK_q:
            {
                show_panel(PANEL_INVENTORY);
                inventory_action = INVENTORY_ACTION_QUAFF;

                world_log(
                    world->player->floor,
                    world->player->x,
                    world->player->y,
                    color_yellow,
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
                    show_panel(PANEL_INVENTORY);
                    inventory_action = INVENTORY_ACTION_READ;

                    world_log(
                        world->player->floor,
                        world->player->x,
                        world->player->y,
                        color_yellow,
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
                    color_yellow,
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
                show_panel(PANEL_CHARACTER);
                character_action = CHARACTER_ACTION_EQUIPMENT_UNEQUIP;

                world_log(
                    world->player->floor,
                    world->player->x,
                    world->player->y,
                    color_yellow,
                    "Choose an item to unequip. Press 'ESC' to cancel.");
            }
            break;
            case SDLK_x:
            {
                if (event->key.keysym.mod & KMOD_SHIFT)
                {
                    show_panel(PANEL_INVENTORY);
                    inventory_action = INVENTORY_ACTION_EXAMINE;

                    world_log(
                        world->player->floor,
                        world->player->x,
                        world->player->y,
                        color_yellow,
                        "Choose an item to examine. Press 'ESC' to cancel.");
                }
                else
                {
                    if (event->key.keysym.mod & KMOD_CTRL)
                    {
                        show_panel(PANEL_CHARACTER);
                        character_action = CHARACTER_ACTION_EQUIPMENT_EXAMINE;

                        world_log(
                            world->player->floor,
                            world->player->x,
                            world->player->y,
                            color_yellow,
                            "Choose an equipment to examine. Press 'ESC' to cancel.");
                    }
                    else
                    {
                        if (targeting_action == TARGETING_ACTION_EXAMINE)
                        {
                            show_panel(PANEL_EXAMINE); // TODO: send examine target to ui
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
                    show_panel(PANEL_SPELLBOOK);
                    spellbook_action = SPELLBOOK_ACTION_SELECT;

                    world_log(
                        world->player->floor,
                        world->player->x,
                        world->player->y,
                        color_yellow,
                        "Choose a spell. Press 'ESC' to cancel.");
                }
                else
                {
                    if (world->player->readied_spell_type != SPELL_TYPE_NONE)
                    {
                        const enum spell_range spell_range = spell_database[world->player->readied_spell_type].range;
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
            }
        }
        break;
        }
    }
    break;
    case SDL_MOUSEBUTTONDOWN:
    {
        if (event->button.button == SDL_BUTTON_LEFT)
        {
            automove_action = AUTOMOVE_ACTION_NONE;

            if (tooltip_rect.visible)
            {
                if (rect_is_inside(tooltip_rect, mouse_x, mouse_y))
                {
                    const struct tooltip_option *const tooltip_option = tooltip_option_mouseover();

                    if (tooltip_option)
                    {
                        if (tooltip_option->on_click)
                        {
                            world->player->took_turn = tooltip_option->on_click();
                        }

                        hide_tooltip();
                    }
                }
                else
                {
                    hide_tooltip();
                }
            }
            else if (rect_is_inside(panel_rect, mouse_x, mouse_y))
            {
                if (inventory_action != INVENTORY_ACTION_NONE)
                {
                    struct item *const item = panel_inventory_item_mouseover();

                    if (item)
                    {
                        world->player->took_turn = do_inventory_action(item);
                    }
                }
                else if (character_action != CHARACTER_ACTION_NONE)
                {
                    if (character_action == CHARACTER_ACTION_ABILITY_ADD_POINT)
                    {
                        const enum ability ability = panel_character_ability_mouseover();

                        if (ability != -1)
                        {
                            world->player->took_turn = do_character_action_ability(ability);
                        }
                    }
                    else if (character_action == CHARACTER_ACTION_EQUIPMENT_EXAMINE ||
                             character_action == CHARACTER_ACTION_EQUIPMENT_UNEQUIP)
                    {
                        const enum equip_slot equip_slot = panel_character_equip_slot_mouseover();

                        if (equip_slot != EQUIP_SLOT_NONE)
                        {
                            world->player->took_turn = do_character_action_equipment(equip_slot);
                        }
                    }
                }
                else if (spellbook_action != SPELLBOOK_ACTION_NONE)
                {
                    const enum spell_type spell_type = panel_spellbook_spell_type_mouseover();

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
                    if (event->key.keysym.mod & KMOD_CTRL)
                    {
                        if (actor_has_ranged_weapon(world->player))
                        {
                            world->player->took_turn = actor_shoot(world->player, mouse_tile_x, mouse_tile_y);
                        }
                        else
                        {
                            const float angle = angle_between(world->player->x, world->player->y, mouse_tile_x, mouse_tile_y);
                            const enum direction direction = get_direction_from_angle(angle);

                            world->player->took_turn = player_swing(direction);
                        }
                    }
                    else
                    {
                        automove_action = AUTOMOVE_ACTION_MOVE;
                        automove_x = mouse_tile_x;
                        automove_y = mouse_tile_y;
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
                    const enum ability ability = panel_character_ability_mouseover();

                    if (ability != -1)
                    {
                        show_tooltip();

                        tooltip_data.ability = ability;

                        add_tooltip_option("Add Point", &on_click_add_point);

                        add_tooltip_option("Cancel", NULL);
                    }

                    const enum equip_slot equip_slot = panel_character_equip_slot_mouseover();

                    if (equip_slot != EQUIP_SLOT_NONE)
                    {
                        const struct item *const equipment = world->player->equipment[equip_slot];

                        if (equipment)
                        {
                            show_tooltip();

                            tooltip_data.equip_slot = equip_slot;

                            add_tooltip_option("Unequip", &on_click_unequip);

                            add_tooltip_option("Cancel", NULL);
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
                    struct item *const item = panel_inventory_item_mouseover();

                    if (item)
                    {
                        show_tooltip();

                        tooltip_data.item = item;

                        add_tooltip_option("Drop", &on_click_drop);

                        const struct item_data *const item_data = &item_database[item->type];
                        const struct base_item_data *const base_item_data = &base_item_database[item_data->type];

                        if (base_item_data->equip_slot != EQUIP_SLOT_NONE)
                        {
                            add_tooltip_option("Equip", &on_click_equip);
                        }

                        if (item_data->type == BASE_ITEM_TYPE_POTION)
                        {
                            add_tooltip_option("Quaff", &on_click_quaff);
                        }

                        add_tooltip_option("Cancel", NULL);
                    }
                }
                break;
                case PANEL_SPELLBOOK:
                {
                    const enum spell_type spell_type = panel_spellbook_spell_type_mouseover();
                    if (spell_type != SPELL_TYPE_NONE)
                    {
                        show_tooltip();

                        tooltip_data.spell_type = spell_type;

                        if (world->player->readied_spell_type == spell_type)
                        {
                            add_tooltip_option("Unready", &on_click_unready);
                        }
                        else
                        {
                            add_tooltip_option("Ready", &on_click_ready);
                        }

                        add_tooltip_option("Cancel", NULL);
                    }
                }
                break;
                }
            }
            else if (map_is_inside(mouse_tile_x, mouse_tile_y))
            {
                show_tooltip();

                tooltip_data.x = mouse_tile_x;
                tooltip_data.y = mouse_tile_y;

                const struct map *const map = &world->maps[world->player->floor];
                const struct tile *const tile = &map->tiles[mouse_tile_x][mouse_tile_y];

                add_tooltip_option("Move", &on_click_move);

                add_tooltip_option("Shoot", &on_click_shoot); // TODO: only if ranged weapon equipped

                if (tile->object)
                {
                    tooltip_data.object = tile->object;

                    add_tooltip_option("Examine Object", &on_click_examine);
                    add_tooltip_option("Interact", &on_click_interact);
                    add_tooltip_option("Bash", &on_click_bash);
                }

                if (tile->actor)
                {
                    tooltip_data.actor = tile->actor;

                    add_tooltip_option("Examine Actor", &on_click_examine);
                    add_tooltip_option("Swap", &on_click_swap);
                    add_tooltip_option("Attack", &on_click_attack);

                    if (tile->actor == world->player)
                    {
                        add_tooltip_option("Character Sheet", &on_click_character_sheet);
                        add_tooltip_option("Inventory", &on_click_inventory);
                        add_tooltip_option("Spellbook", &on_click_spellbook);
                    }
                }

                struct item *const item = list_get(tile->items, 0);
                if (item)
                {
                    tooltip_data.item = item;

                    add_tooltip_option("Examine Item", NULL);
                    add_tooltip_option("Take Item", NULL);
                }

                if (tile->items->size > 1)
                {
                    add_tooltip_option("Take All", NULL);
                }

                add_tooltip_option("Cancel", NULL);
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

    if (automove_action != AUTOMOVE_ACTION_NONE && world_player_can_take_turn())
    {
        if (automove_actor)
        {
            automove_x = automove_actor->x;
            automove_y = automove_actor->y;
        }

        if (automove_object)
        {
            automove_x = automove_object->x;
            automove_y = automove_object->y;
        }

        if (distance_between(
                world->player->x, world->player->y,
                automove_x, automove_y) < 2.0f)
        {
        }

        switch (automove_action)
        {
        case AUTOMOVE_ACTION_ATTACK:
        {
            if (actor_has_ranged_weapon(world->player))
            {
                world->player->took_turn = actor_shoot(world->player, automove_x, automove_y);
            }
            else
            {
                world->player->took_turn = actor_attack(world->player, automove_actor, NULL);
            }
        }
        break;
        case AUTOMOVE_ACTION_BASH:
        {
            world->player->took_turn = actor_bash(world->player, automove_object);
        }
        break;
        case AUTOMOVE_ACTION_INTERACT:
        {
            world->player->took_turn = actor_interact(world->player, automove_object);
        }
        break;
        case AUTOMOVE_ACTION_MOVE:
        {
            world->player->took_turn = actor_path_towards(world->player, automove_x, automove_y);
        }
        break;
        case AUTOMOVE_ACTION_SWAP:
        {
            world->player->took_turn = actor_swap(world->player, automove_actor);
        }
        break;
        }

        // if automove action failed, stop automoving
        if (!world->player->took_turn)
        {
            automove_action = AUTOMOVE_ACTION_NONE;
        }

        // stop automoving if there is an enemy in FOV
        if (actor_is_enemy_nearby(world->player))
        {
            automove_action = AUTOMOVE_ACTION_NONE;
        }

        // stop automoving if the target actor is dead
        if (automove_actor && automove_actor->dead)
        {
            automove_action = AUTOMOVE_ACTION_NONE;
        }

        // TODO: stop automoving if the target object is destroyed
    }
    else
    {
        automove_actor = NULL;
        automove_object = NULL;
    }

    world_update(delta_time);

    // delete save if hero died
    if (world->hero->dead && file_exists(SAVE_PATH))
    {
        file_delete(SAVE_PATH);
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
        for (size_t tooltip_option_index = 0; tooltip_option_index < tooltip_options->size; ++tooltip_option_index)
        {
            const struct tooltip_option *const option = list_get(tooltip_options, tooltip_option_index);

            const int len = (int)strlen(option->text) + 2;
            if (len > tooltip_rect.width)
            {
                tooltip_rect.width = len;
            }
        }

        // tooltip should be as tall as the number of options, plus 2 for borders
        // TODO: multi-line options?
        tooltip_rect.height = (int)tooltip_options->size + 2;

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
        const float dx = TCOD_noise_get(noise, &noise_dx) * 0.5f;
        noise_dx += 30;
        const float dy = TCOD_noise_get(noise, &noise_dx) * 0.5f;
        const float di = 0.2f * TCOD_noise_get(noise, &noise_x);

        // get map to draw
        const struct map *const map = &world->maps[world->player->floor];

        // draw tiles
        for (int x = view_rect.x; x < view_rect.x + view_rect.width; x++)
        {
            for (int y = view_rect.y; y < view_rect.y + view_rect.height; y++)
            {
                if (map_is_inside(x, y))
                {
                    const struct tile *const tile = &map->tiles[x][y];
                    const struct tile_data tile_data = tile_database[tile->type];

                    // ambient lighting
                    float fg_r = tile_common.ambient_light_color.r;
                    float fg_g = tile_common.ambient_light_color.g;
                    float fg_b = tile_common.ambient_light_color.b;
                    float bg_r = fg_r * tile_common.ambient_light_intensity;
                    float bg_g = fg_g * tile_common.ambient_light_intensity;
                    float bg_b = fg_b * tile_common.ambient_light_intensity;

                    if (TCOD_map_is_in_fov(world->player->fov, x, y))
                    {
                        // calculate object lighting
                        for (size_t object_index = 0; object_index < map->objects->size; object_index++)
                        {
                            const struct object *const object = list_get(map->objects, object_index);

                            if (object->light_fov && TCOD_map_is_in_fov(object->light_fov, x, y))
                            {
                                const struct object_data *const object_data = &object_database[object->type];
                                const struct light_data *const light_data = &light_database[object_data->light_type];

                                const float radius_sq = powf((float)light_data->radius, 2);
                                const float distance_sq =
                                    powf((float)(x - object->x + (light_data->flicker ? dx : 0)), 2) +
                                    powf((float)(y - object->y + (light_data->flicker ? dy : 0)), 2);
                                const float attenuation = CLAMP(
                                    0.0f,
                                    1.0f,
                                    (radius_sq - distance_sq) / radius_sq + (light_data->flicker ? di : 0));

                                fg_r += light_data->color.r * attenuation;
                                fg_g += light_data->color.g * attenuation;
                                fg_b += light_data->color.b * attenuation;
                                bg_r += fg_r * light_data->intensity * attenuation;
                                bg_g += fg_g * light_data->intensity * attenuation;
                                bg_b += fg_b * light_data->intensity * attenuation;
                            }
                        }

                        // calculate actor lighting
                        for (size_t actor_index = 0; actor_index < map->actors->size; actor_index++)
                        {
                            const struct actor *const actor = list_get(map->actors, actor_index);

                            if (actor->light_fov && TCOD_map_is_in_fov(actor->light_fov, x, y))
                            {
                                const struct light_data *const light_data = &light_database[actor->light_type];

                                const float radius_sq = powf((float)light_data->radius, 2);
                                const float distance_sq =
                                    powf((float)(x - actor->x + (light_data->flicker ? dx : 0)), 2) +
                                    powf((float)(y - actor->y + (light_data->flicker ? dy : 0)), 2);
                                const float attenuation = CLAMP(
                                    0.0f,
                                    1.0f,
                                    (radius_sq - distance_sq) / radius_sq + (light_data->flicker ? di : 0));

                                fg_r += light_data->color.r * attenuation;
                                fg_g += light_data->color.g * attenuation;
                                fg_b += light_data->color.b * attenuation;
                                bg_r += light_data->color.r * light_data->intensity * attenuation;
                                bg_g += light_data->color.g * light_data->intensity * attenuation;
                                bg_b += light_data->color.b * light_data->intensity * attenuation;
                            }
                        }

                        // calculate projectile lighting
                        for (size_t projectile_index = 0; projectile_index < map->projectiles->size; projectile_index++)
                        {
                            const struct projectile *const projectile = list_get(map->projectiles, projectile_index);

                            if (projectile->light_fov && TCOD_map_is_in_fov(projectile->light_fov, x, y))
                            {
                                const struct projectile_data *const projectile_data = &projectile_database[projectile->type];
                                const struct light_data *const light_data = &light_database[projectile_data->light_type];

                                const float radius_sq = powf((float)light_data->radius, 2);
                                const float distance_sq =
                                    powf((float)(x - projectile->x + (light_data->flicker ? dx : 0)), 2) +
                                    powf((float)(y - projectile->y + (light_data->flicker ? dy : 0)), 2);
                                const float attenuation = CLAMP(
                                    0.0f,
                                    1.0f,
                                    (radius_sq - distance_sq) / radius_sq + (light_data->flicker ? di : 0));

                                fg_r += light_data->color.r * attenuation;
                                fg_g += light_data->color.g * attenuation;
                                fg_b += light_data->color.b * attenuation;
                                bg_r += light_data->color.r * light_data->intensity * attenuation;
                                bg_g += light_data->color.g * light_data->intensity * attenuation;
                                bg_b += light_data->color.b * light_data->intensity * attenuation;
                            }
                        }

                        // calculate explosion lighting
                        for (size_t explosion_index = 0; explosion_index < map->explosions->size; explosion_index++)
                        {
                            const struct explosion *const explosion = list_get(map->explosions, explosion_index);

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
                    const float fg_max = MAX(fg_r, MAX(fg_g, fg_b));
                    const float fg_mult = fg_max > 255.0f ? 255.0f / fg_max : 1.0f;
                    const TCOD_ColorRGB fg_color = TCOD_color_multiply(
                        TCOD_color_RGB(
                            (uint8_t)(fg_r * fg_mult),
                            (uint8_t)(fg_g * fg_mult),
                            (uint8_t)(fg_b * fg_mult)),
                        tile_data.color);
                    const float bg_max = MAX(bg_r, MAX(bg_g, bg_b));
                    const float bg_mult = bg_max > 255.0f ? 255.0f / bg_max : 1.0f;
                    const TCOD_ColorRGB bg_color = TCOD_color_multiply(
                        TCOD_color_RGB(
                            (uint8_t)(bg_r * bg_mult),
                            (uint8_t)(bg_g * bg_mult),
                            (uint8_t)(bg_b * bg_mult)),
                        tile_data.color);

                    if (tile->explored)
                    {
                        int glyph = tile_data.glyph;

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
                                0x253C, // 15 - NESW = ┼
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
        for (size_t corpse_index = 0; corpse_index < map->corpses->size; corpse_index++)
        {
            const struct corpse *const corpse = list_get(map->corpses, corpse_index);

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
        for (size_t object_index = 0; object_index < map->objects->size; object_index++)
        {
            const struct object *const object = list_get(map->objects, object_index);

            if (object->type != OBJECT_TYPE_STAIR_DOWN && object->type != OBJECT_TYPE_STAIR_UP &&
                TCOD_map_is_in_fov(world->player->fov, object->x, object->y))
            {
                const struct object_data *const object_data = &object_database[object->type];

                TCOD_console_set_char_foreground(
                    console,
                    object->x - view_rect.x,
                    object->y - view_rect.y,
                    object_data->color);
                TCOD_console_set_char(
                    console,
                    object->x - view_rect.x,
                    object->y - view_rect.y,
                    object_data->glyph);
            }
        }

        // draw items
        for (size_t item_index = 0; item_index < map->items->size; item_index++)
        {
            const struct item *const item = list_get(map->items, item_index);

            if (TCOD_map_is_in_fov(world->player->fov, item->x, item->y))
            {
                const struct item_data *const item_data = &item_database[item->type];
                const struct base_item_data *const base_item_data = &base_item_database[item_data->type];

                TCOD_console_set_char_foreground(
                    console,
                    item->x - view_rect.x,
                    item->y - view_rect.y,
                    item_data->color);
                TCOD_console_set_char(
                    console,
                    item->x - view_rect.x,
                    item->y - view_rect.y,
                    base_item_data->glyph);
            }
        }

        // draw projectiles
        for (size_t projectile_index = 0; projectile_index < map->projectiles->size; projectile_index++)
        {
            const struct projectile *const projectile = list_get(map->projectiles, projectile_index);

            const int x = (int)projectile->x;
            const int y = (int)projectile->y;

            if (TCOD_map_is_in_fov(world->player->fov, x, y))
            {
                char glyph = projectile_database[projectile->type].glyph;

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

                    const float angle = angle_between(projectile->origin_x, projectile->origin_y, projectile->target_x, projectile->target_y);
                    const enum direction direction = get_direction_from_angle(angle);

                    glyph = glyphs[direction];
                }

                TCOD_console_set_char_foreground(
                    console,
                    x - view_rect.x,
                    y - view_rect.y,
                    projectile_database[projectile->type].color);
                TCOD_console_set_char(
                    console,
                    x - view_rect.x,
                    y - view_rect.y,
                    glyph);
            }
        }

        // draw stairs (to make sure they are drawn on top of other entities)
        for (size_t object_index = 0; object_index < map->objects->size; object_index++)
        {
            const struct object *const object = list_get(map->objects, object_index);

            if ((object->type == OBJECT_TYPE_STAIR_DOWN || object->type == OBJECT_TYPE_STAIR_UP) &&
                TCOD_map_is_in_fov(world->player->fov, object->x, object->y))
            {
                const struct object_data *const object_data = &object_database[object->type];

                TCOD_console_set_char_foreground(
                    console,
                    object->x - view_rect.x,
                    object->y - view_rect.y,
                    object_data->color);
                TCOD_console_set_char(
                    console,
                    object->x - view_rect.x,
                    object->y - view_rect.y,
                    object_data->glyph);
            }
        }

        // draw actors
        for (size_t actor_index = 0; actor_index < map->actors->size; actor_index++)
        {
            const struct actor *const actor = list_get(map->actors, actor_index);

            if (TCOD_map_is_in_fov(world->player->fov, actor->x, actor->y))
            {
                TCOD_ColorRGB color = class_database[actor->class].color;
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
                    race_database[actor->race].glyph);
            }
        }

        // draw targeting reticle
        if (targeting_action != TARGETING_ACTION_NONE)
        {
            TCOD_console_set_char_foreground(
                console,
                target_x - 1 - view_rect.x,
                target_y - view_rect.y,
                color_red);
            TCOD_console_set_char(
                console,
                target_x - 1 - view_rect.x,
                target_y - view_rect.y,
                '[');
            TCOD_console_set_char_foreground(
                console,
                target_x + 1 - view_rect.x,
                target_y - view_rect.y,
                color_red);
            TCOD_console_set_char(
                console,
                target_x + 1 - view_rect.x,
                target_y - view_rect.y,
                ']');

            // descriptive text of target
            if (map_is_inside(target_x, target_y))
            {
                const struct tile *const tile = &map->tiles[target_x][target_y];

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

                    const struct corpse *const corpse = list_get(tile->corpses, 0);
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

                    const struct item *const item = list_get(tile->items, 0);
                    if (item)
                    {
                        if (tile->items->size > 1)
                        {
                            TCOD_console_printf_ex(
                                console,
                                view_rect.width / 2,
                                view_rect.height - 2,
                                TCOD_BKGND_NONE,
                                TCOD_CENTER,
                                "%s (multiple)",
                                item_database[item->type].name);
                        }
                        else
                        {
                            TCOD_console_printf_ex(
                                console,
                                view_rect.width / 2,
                                view_rect.height - 2,
                                TCOD_BKGND_NONE,
                                TCOD_CENTER,
                                item_database[item->type].name);
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
                            object_database[tile->object->type].name);

                        goto done;
                    }

                    TCOD_console_printf_ex(
                        console,
                        view_rect.width / 2,
                        view_rect.height - 2,
                        TCOD_BKGND_NONE,
                        TCOD_CENTER,
                        tile_database[tile->type].name);
                done:;
                }
                else
                {
                    if (tile->explored)
                    {
                        TCOD_console_printf_ex(
                            console,
                            view_rect.width / 2,
                            view_rect.height - 2,
                            TCOD_BKGND_NONE,
                            TCOD_CENTER,
                            "%s (hidden)",
                            tile_database[tile->type].name);
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
                const TCOD_ColorRGB fg =
                    (float)world->player->hit_points / max_health > 0.5f
                        ? TCOD_color_lerp(color_yellow, color_green, world->player->hit_points / (max_health * 0.5f))
                        : TCOD_color_lerp(color_red, color_yellow, world->player->hit_points / (max_health * 0.5f));

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
                const TCOD_ColorRGB fg = TCOD_color_lerp(color_gray, color_azure, (float)world->player->mana_points / max_mana);

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
                &color_white,
                NULL,
                TCOD_BKGND_NONE,
                TCOD_LEFT,
                "Floor: %d",
                world->player->floor);

            console_print(
                status_rect.console,
                1,
                y++,
                &color_white,
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
            int message_index = (int)world->messages->size - 1;
            while (message_index >= 0)
            {
                const struct message *const message = list_get(world->messages, message_index--);

                if (!message)
                {
                    break;
                }

                const int lines = TCOD_console_get_height_rect_fmt(
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

                const TCOD_ColorRGB fg = TCOD_color_lerp(color_gray, message->color, (float)y / (message_log_rect.height - 2));

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

        const struct panel_state *const current_panel_status = &panel_state[current_panel];
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
                race_database[world->player->race].name);
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
                class_database[world->player->class].name);
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
                const TCOD_ColorRGB fg =
                    ability == panel_character_ability_mouseover()
                        ? color_yellow
                        : color_white;

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
                        ability_database[ability].name);
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
                        ability_database[ability].name);
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
                const TCOD_ColorRGB fg =
                    equip_slot == panel_character_equip_slot_mouseover()
                        ? color_yellow
                        : color_white;
                const struct equip_slot_data equip_slot_data = equip_slot_database[equip_slot];

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
                        equip_slot_data.name);
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
                        equip_slot_data.name);
                }

                if (world->player->equipment[equip_slot])
                {
                    const struct item *const equipment = world->player->equipment[equip_slot];
                    const struct item_data equipment_data = item_database[equipment->type];

                    console_print(
                        panel_rect.console,
                        panel_rect.width - 2,
                        y - current_panel_status->scroll,
                        &fg,
                        NULL,
                        TCOD_BKGND_NONE,
                        TCOD_RIGHT,
                        equipment->stack > 1 ? "%s (%d)" : "%s",
                        equipment_data.name,
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
            const struct item *const mouseover_item = panel_inventory_item_mouseover();
            int y = 1;
            for (size_t item_index = 0; item_index < world->player->items->size; item_index++)
            {
                const struct item *const item = list_get(world->player->items, item_index);
                const struct item_data item_data = item_database[item->type];

                const TCOD_ColorRGB fg =
                    item == mouseover_item
                        ? color_yellow
                        : item_data.color;

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
                        item_data.name,
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
                        item_data.name,
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
            const enum spell_type mouseover_spell_type = panel_spellbook_spell_type_mouseover();

            int y = 1;
            for (size_t known_spell_type_index = 0; known_spell_type_index < world->player->known_spell_types->size; known_spell_type_index++)
            {
                const enum spell_type spell_type = (size_t)list_get(world->player->known_spell_types, known_spell_type_index);
                const struct spell_data *const spell_data = &spell_database[spell_type];

                const TCOD_ColorRGB fg =
                    spell_type == mouseover_spell_type
                        ? color_yellow
                        : color_white;

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
                        spell_data->name);
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
                        spell_data->name);
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
                    spell_data->mana_cost);

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
        for (size_t tooltip_option_index = 0; tooltip_option_index < tooltip_options->size; tooltip_option_index++)
        {
            const struct tooltip_option *const option = list_get(tooltip_options, tooltip_option_index);

            const TCOD_ColorRGB fg =
                option == mouseover_tooltip_option
                    ? color_yellow
                    : color_white;

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

    if (automove_action != AUTOMOVE_ACTION_NONE)
    {
        TCOD_console_set_char_background(console, automove_x - view_rect.x, automove_y - view_rect.y, color_red, TCOD_BKGND_SET);
    }

    if (!world->hero->dead && !world_player_can_take_turn())
    {
        console_print(
            console,
            0, 0,
            &color_white,
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
