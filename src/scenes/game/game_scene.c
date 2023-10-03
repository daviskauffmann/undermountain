#include "game_scene.h"

#include "../../config.h"
#include "../../game/actor.h"
#include "../../game/base_item.h"
#include "../../game/class.h"
#include "../../game/color.h"
#include "../../game/corpse.h"
#include "../../game/explosion.h"
#include "../../game/feat.h"
#include "../../game/item.h"
#include "../../game/light.h"
#include "../../game/list.h"
#include "../../game/message.h"
#include "../../game/object.h"
#include "../../game/projectile.h"
#include "../../game/race.h"
#include "../../game/spell.h"
#include "../../game/spell_range.h"
#include "../../game/surface.h"
#include "../../game/util.h"
#include "../../game/world.h"
#include "../../print.h"
#include "../../scene.h"
#include "../../util.h"
#include "../menu/menu_scene.h"
#include "character_action.h"
#include "direction.h"
#include "directional_action.h"
#include "inventory_action.h"
#include "panel.h"
#include "rect.h"
#include "spellbook_action.h"
#include "targeting_action.h"
#include <float.h>
#include <libtcod.h>
#include <malloc.h>
#include <math.h>
#include <stdio.h>
#include <string.h>

// TODO: prompts

/* Targeting */

static enum targeting_action targeting_action;
static int target_x;
static int target_y;
static struct item *targeting_item;

/* Viewport */

static struct rect view_rect;

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

/* Directional actions */

static enum directional_action directional_action;

static bool do_directional_action(const enum direction direction)
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
    case DIRECTIONAL_ACTION_DIP:
    {
        took_turn = actor_dip(world->player, x, y);
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
        took_turn = world_can_player_take_turn() && actor_drop(world->player, item);
    }
    break;
    case INVENTORY_ACTION_EQUIP:
    {
        took_turn = world_can_player_take_turn() && actor_equip(world->player, item);
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
        took_turn = world_can_player_take_turn() && actor_quaff(world->player, item);
    }
    break;
    case INVENTORY_ACTION_READ:
    {
        bool needs_target = false;

        const struct item_data *const item_data = &item_database[item->type];
        if (item_data->type == BASE_ITEM_TYPE_SCROLL)
        {
            const struct spell_data *const spell_data = &spell_database[item_data->spell_type];
            if (spell_data->range == SPELL_RANGE_TOUCH)
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
            took_turn = world_can_player_take_turn() && actor_read(world->player, item, world->player->x, world->player->y);
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
            actor_spend_ability_point(world->player, ability);
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
        took_turn = world_can_player_take_turn() && actor_unequip(world->player, equip_slot);
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
        world->player->readied_spell = spell_type;

        world_log(
            world->player->floor,
            world->player->x,
            world->player->y,
            color_purple,
            "%s readied.",
            spell_database[spell_type].name);

        took_turn = true;
    }
    break;
    }

    spellbook_action = SPELLBOOK_ACTION_NONE;

    return took_turn;
}

/* Noise */

static TCOD_noise_t noise;
static float noise_x;

/* Graphics settings */
static bool complex_lighting = true;

static void init(const struct scene *const previous_scene)
{
    previous_scene;

    view_rect.console = NULL;
    view_rect.visible = true;

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
    if (!world->doomed)
    {
        FILE *const file = fopen(SAVE_PATH, "wb");
        world_save(file);
        fclose(file);
    }
    world_uninit();

    TCOD_noise_delete(noise);

    TCOD_console_delete(panel_rect.console);

    TCOD_console_delete(message_log_rect.console);

    TCOD_console_delete(status_rect.console);

    TCOD_console_delete(hud_rect.console);
}

static bool player_swing(const enum direction direction)
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

static bool player_interact(const SDL_Event *const event, const enum direction direction)
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

struct scene *handle_event(const SDL_Event *event)
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
        // determine if player is selecting something in a list
        // if so, don't process any other keyboard input
        bool handled = false;

        if (event->key.keysym.sym >= SDLK_a && event->key.keysym.sym <= SDLK_z)
        {
            const int alpha = event->key.keysym.sym - SDLK_a;

            if (inventory_action != INVENTORY_ACTION_NONE &&
                alpha >= 0 && alpha < world->player->items->size)
            {
                if (world_can_player_take_turn())
                {
                    struct item *const item = list_get(world->player->items, alpha);

                    world->player->took_turn = do_inventory_action(item);
                }

                handled = true;
            }
            else if ((character_action == CHARACTER_ACTION_ABILITY_ADD_POINT) &&
                     alpha >= 0 && alpha < NUM_ABILITIES - 1)
            {
                if (world_can_player_take_turn())
                {
                    const enum ability_type ability = alpha + 1;

                    world->player->took_turn = do_character_action_ability(ability);
                }

                handled = true;
            }
            else if ((character_action == CHARACTER_ACTION_EQUIPMENT_EXAMINE ||
                      character_action == CHARACTER_ACTION_EQUIPMENT_UNEQUIP) &&
                     alpha >= 0 && alpha < NUM_EQUIP_SLOTS - 1)
            {
                if (world_can_player_take_turn())
                {
                    const enum equip_slot equip_slot = alpha + 1;

                    world->player->took_turn = do_character_action_equipment(equip_slot);
                }

                handled = true;
            }
            else if (spellbook_action != SPELLBOOK_ACTION_NONE)
            {
                if (world_can_player_take_turn())
                {
                    enum spell_type spell_type = SPELL_TYPE_NONE;

                    bool spells[NUM_SPELL_TYPES] = {false};
                    actor_calc_spells(world->player, &spells);
                    int spell_count = 0;
                    for (enum spell_type _spell_type = SPELL_TYPE_NONE + 1; _spell_type < NUM_SPELL_TYPES; _spell_type++)
                    {
                        if (spells[_spell_type])
                        {
                            spell_count++;

                            if (alpha + 1 == spell_count)
                            {
                                spell_type = _spell_type;
                                break;
                            }
                        }
                    }

                    if (spell_type != SPELL_TYPE_NONE)
                    {
                        world->player->took_turn = do_spellbook_action(spell_type);
                    }
                }

                handled = true;
            }
        }

        if (handled)
        {
            break;
        }

        switch (event->key.keysym.sym)
        {
        case SDLK_ESCAPE:
        {
            if (directional_action != DIRECTIONAL_ACTION_NONE ||
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
        case SDLK_COMMA:
        {
            if (event->key.keysym.mod & KMOD_SHIFT)
            {
                world->player->took_turn = world_can_player_take_turn() && actor_ascend(world->player);
            }
        }
        break;
        case SDLK_PERIOD:
        {
            if (event->key.keysym.mod & KMOD_SHIFT)
            {
                world->player->took_turn = world_can_player_take_turn() && actor_descend(world->player);
            }
        }
        break;
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
                    "Choose a direction to close door. Press 'ESC' to cancel.");
            }
        }
        break;
        case SDLK_d:
        {
            if (event->key.keysym.mod & KMOD_SHIFT)
            {
                toggle_panel(PANEL_CHARACTER);
                directional_action = DIRECTIONAL_ACTION_DIP;

                world_log(
                    world->player->floor,
                    world->player->x,
                    world->player->y,
                    color_yellow,
                    "Choose a direction to dip. Press 'ESC' to cancel.");
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
                if (world_can_player_take_turn())
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
            world->player->took_turn = world_can_player_take_turn() && actor_grab(world->player, world->player->x, world->player->y);
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
            if (event->key.keysym.mod & KMOD_SHIFT)
            {
                world->player->took_turn = world_can_player_take_turn() && actor_rest(world->player);
            }
            else
            {
                if (targeting_action == TARGETING_ACTION_READ)
                {
                    if (world_can_player_take_turn())
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
        }
        break;
        case SDLK_t:
        {
            if (event->key.keysym.mod & KMOD_SHIFT)
            {
                if (world_can_player_take_turn())
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
                if (world_can_player_take_turn())
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
                if (world->player->readied_spell != SPELL_TYPE_NONE)
                {
                    const enum spell_range spell_range = spell_database[world->player->readied_spell].range;

                    switch (spell_range)
                    {
                    case SPELL_RANGE_PERSONAL:
                    {
                        world->player->took_turn =
                            world_can_player_take_turn() &&
                            actor_cast(
                                world->player,
                                world->player->readied_spell,
                                world->player->x, world->player->y,
                                true);
                    }
                    break;
                    case SPELL_RANGE_TOUCH:
                    {
                        if (targeting_action == TARGETING_ACTION_SPELL)
                        {
                            if (world_can_player_take_turn())
                            {
                                world->player->took_turn = actor_cast(
                                    world->player,
                                    world->player->readied_spell,
                                    target_x, target_y,
                                    true);

                                targeting_action = TARGETING_ACTION_NONE;
                            }
                        }
                        else
                        {
                            targeting_action = TARGETING_ACTION_SPELL;

                            if (spell_database[world->player->readied_spell].harmful)
                            {
                                const struct actor *nearest_enemy = actor_find_nearest_enemy(world->player);

                                if (!nearest_enemy)
                                {
                                    nearest_enemy = world->player;
                                }

                                target_x = nearest_enemy->x;
                                target_y = nearest_enemy->y;
                            }
                            else
                            {
                                target_x = world->player->x;
                                target_y = world->player->y;
                            }
                        }
                    }
                    break;
                    }
                }
            }
        }
        break;
        case SDLK_F1:
        {
            complex_lighting = !complex_lighting;
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
        case SDLK_PAGEDOWN:
        {
            if (panel_rect.visible)
            {
                panel_state[current_panel].scroll++;
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
            else
            {
                world->player->took_turn = world_can_player_take_turn() && player_interact(event, DIRECTION_SW);
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
            else
            {
                world->player->took_turn = world_can_player_take_turn() && player_interact(event, DIRECTION_S);
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
            else
            {
                world->player->took_turn = world_can_player_take_turn() && player_interact(event, DIRECTION_SE);
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
            else
            {
                world->player->took_turn = world_can_player_take_turn() && player_interact(event, DIRECTION_W);
            }
        }
        break;
        case SDLK_KP_5:
        {
            world->player->took_turn = world_can_player_take_turn();
        }
        break;
        case SDLK_KP_6:
        case SDLK_RIGHT:
        {
            if (targeting_action != TARGETING_ACTION_NONE)
            {
                target_x++;
            }
            else
            {
                world->player->took_turn = world_can_player_take_turn() && player_interact(event, DIRECTION_E);
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
            else
            {
                world->player->took_turn = world_can_player_take_turn() && player_interact(event, DIRECTION_NW);
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
            else
            {
                world->player->took_turn = world_can_player_take_turn() && player_interact(event, DIRECTION_N);
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
            else
            {
                world->player->took_turn = world_can_player_take_turn() && player_interact(event, DIRECTION_NE);
            }
        }
        break;
        }
    }
    break;
    }

    return &game_scene;
}

static int get_wall_glyph(const struct map *const map, const int x, const int y)
{
    static const int glyphs[] = {
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

    return glyphs[index];
}

static struct scene *
update(TCOD_Console *const console, const float delta_time)
{
    if (!world->player)
    {
        world->player = world->hero;
        if (!world->player)
        {
            return &game_scene;
        }
    }

    world_update(delta_time);

    // delete save if hero died
    if (world->doomed && file_exists(SAVE_PATH))
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

        // get map to draw
        const struct map *const map = &world->maps[world->player->floor];

        if (complex_lighting)
        {
            // calculate random noise coefficients
            noise_x += delta_time * 10;
            float noise_dx = noise_x + 20;
            const float dx = TCOD_noise_get(noise, &noise_dx) * 0.5f;
            noise_dx += 30;
            const float dy = TCOD_noise_get(noise, &noise_dx) * 0.5f;
            const float di = 0.2f * TCOD_noise_get(noise, &noise_x);

            // draw tiles
            for (int x = view_rect.x; x < view_rect.x + view_rect.width; x++)
            {
                for (int y = view_rect.y; y < view_rect.y + view_rect.height; y++)
                {
                    if (!map_is_inside(x, y))
                    {
                        continue;
                    }

                    // ambient lighting
                    float fg_r = tile_metadata.ambient_light_color.r;
                    float fg_g = tile_metadata.ambient_light_color.g;
                    float fg_b = tile_metadata.ambient_light_color.b;
                    float bg_r = fg_r * tile_metadata.ambient_light_intensity;
                    float bg_g = fg_g * tile_metadata.ambient_light_intensity;
                    float bg_b = fg_b * tile_metadata.ambient_light_intensity;

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
                                const struct light_data *light_data = &light_database[actor->light_type];

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
                            const struct explosion_data *const explosion_data = &explosion_database[explosion->type];

                            if (explosion->fov && TCOD_map_is_in_fov(explosion->fov, x, y))
                            {
                                const float radius_sq = powf((float)explosion_data->radius, 2);
                                const float distance_sq =
                                    powf((float)(x - explosion->x + dx * 2), 2) +
                                    powf((float)(y - explosion->y + dy * 2), 2);
                                const float attenuation = CLAMP(
                                    0.0f,
                                    1.0f,
                                    (radius_sq - distance_sq) / radius_sq + di * 2);

                                fg_r += explosion_data->color.r * attenuation;
                                fg_g += explosion_data->color.g * attenuation;
                                fg_b += explosion_data->color.b * attenuation;
                                bg_r += explosion_data->color.r * explosion_data->intensity * attenuation;
                                bg_g += explosion_data->color.g * explosion_data->intensity * attenuation;
                                bg_b += explosion_data->color.b * explosion_data->intensity * attenuation;
                            }
                        }

                        // calculate surface lighting
                        for (size_t surface_index = 0; surface_index < map->surfaces->size; surface_index++)
                        {
                            const struct surface *const surface = list_get(map->surfaces, surface_index);

                            if (surface->light_fov && TCOD_map_is_in_fov(surface->light_fov, x, y))
                            {
                                const struct surface_data *const surface_data = &surface_database[surface->type];
                                const struct light_data *const light_data = &light_database[surface_data->light_type];

                                const float radius_sq = powf((float)light_data->radius, 2);
                                const float distance_sq =
                                    powf((float)(x - surface->x + (light_data->flicker ? dx : 0)), 2) +
                                    powf((float)(y - surface->y + (light_data->flicker ? dy : 0)), 2);
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
                    }

                    // apply tonemapping
                    const struct tile *const tile = &map->tiles[x][y];
                    const struct tile_data *const tile_data = &tile_database[tile->type];

                    const float fg_max = MAX(fg_r, MAX(fg_g, fg_b));
                    const float fg_mult = fg_max > 255 ? 255 / fg_max : 1;
                    TCOD_ColorRGB fg_color = TCOD_color_multiply(
                        TCOD_color_RGB(
                            (uint8_t)(fg_r * fg_mult),
                            (uint8_t)(fg_g * fg_mult),
                            (uint8_t)(fg_b * fg_mult)),
                        tile_data->color);
                    const float bg_max = MAX(bg_r, MAX(bg_g, bg_b));
                    const float bg_mult = bg_max > 255 ? 255 / bg_max : 1;
                    const TCOD_ColorRGB bg_color = TCOD_color_multiply(
                        TCOD_color_RGB(
                            (uint8_t)(bg_r * bg_mult),
                            (uint8_t)(bg_g * bg_mult),
                            (uint8_t)(bg_b * bg_mult)),
                        tile_data->color);

                    if (tile->explored)
                    {
                        int glyph = tile_data->glyph;

                        // select appropriate wall graphic
                        if (tile->type == TILE_TYPE_WALL)
                        {
                            glyph = get_wall_glyph(map, x, y);
                        }

                        if (TCOD_map_is_in_fov(world->player->fov, x, y))
                        {
                            if (tile->actor)
                            {
                                const struct actor *const actor = tile->actor;
                                const struct class_data *const class_data = &class_database[actor->class];

                                TCOD_ColorRGB color = class_data->color;
                                if (actor->flash_fade_coef > 0)
                                {
                                    color = TCOD_color_lerp(color, actor->flash_color, actor->flash_fade_coef);
                                }

                                fg_color = color;
                                glyph = class_data->glyph;

                                goto draw_tile;
                            }

                            if (tile->object)
                            {
                                const struct object_data *const object_data = &object_database[tile->object->type];

                                fg_color = object_data->color;
                                glyph = object_data->glyph;

                                goto draw_tile;
                            }

                            if (tile->item)
                            {
                                const struct item_data *const item_data = &item_database[tile->item->type];
                                const struct base_item_data *const base_item_data = &base_item_database[item_data->type];

                                fg_color = item_data->color;
                                glyph = base_item_data->glyph;

                                goto draw_tile;
                            }

                            if (tile->corpse)
                            {
                                fg_color = corpse_metadata.color;
                                glyph = corpse_metadata.glyph;

                                goto draw_tile;
                            }

                            if (tile->surface)
                            {
                                const struct surface_data *const surface_data = &surface_database[tile->surface->type];

                                fg_color = surface_data->color;

                                if (surface_data->glyph)
                                {
                                    glyph = surface_data->glyph;
                                }

                                goto draw_tile;
                            }
                        }

                    draw_tile:;

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
        else
        {
            // draw tiles
            for (int x = view_rect.x; x < view_rect.x + view_rect.width; x++)
            {
                for (int y = view_rect.y; y < view_rect.y + view_rect.height; y++)
                {
                    if (map_is_inside(x, y))
                    {
                        const struct tile *const tile = &map->tiles[x][y];
                        const struct tile_data *const tile_data = &tile_database[tile->type];

                        if (tile->explored)
                        {
                            int glyph = tile_data->glyph;

                            // select appropriate wall graphic
                            if (tile->type == TILE_TYPE_WALL)
                            {
                                glyph = get_wall_glyph(map, x, y);
                            }

                            TCOD_ColorRGB fg_color = tile_data->color;

                            if (TCOD_map_is_in_fov(world->player->fov, x, y))
                            {
                                if (tile->actor)
                                {
                                    const struct actor *const actor = tile->actor;
                                    const struct class_data *const class_data = &class_database[actor->class];

                                    TCOD_ColorRGB color = class_data->color;
                                    if (actor->flash_fade_coef > 0)
                                    {
                                        color = TCOD_color_lerp(color, actor->flash_color, actor->flash_fade_coef);
                                    }

                                    fg_color = color;
                                    glyph = class_data->glyph;

                                    goto draw_tile_simple;
                                }

                                if (tile->object)
                                {
                                    const struct object_data *const object_data = &object_database[tile->object->type];

                                    fg_color = object_data->color;
                                    glyph = object_data->glyph;

                                    goto draw_tile_simple;
                                }

                                if (tile->item)
                                {
                                    const struct item_data *const item_data = &item_database[tile->item->type];
                                    const struct base_item_data *const base_item_data = &base_item_database[item_data->type];

                                    fg_color = item_data->color;
                                    glyph = base_item_data->glyph;

                                    goto draw_tile_simple;
                                }

                                if (tile->corpse)
                                {
                                    fg_color = corpse_metadata.color;
                                    glyph = corpse_metadata.glyph;

                                    goto draw_tile_simple;
                                }

                                if (tile->surface)
                                {
                                    const struct surface_data *const surface_data = &surface_database[tile->surface->type];

                                    fg_color = surface_data->color;

                                    if (surface_data->glyph)
                                    {
                                        glyph = surface_data->glyph;
                                    }

                                    goto draw_tile_simple;
                                }
                            }

                        draw_tile_simple:;

                            TCOD_console_set_char_foreground(
                                console,
                                x - view_rect.x,
                                y - view_rect.y,
                                TCOD_map_is_in_fov(world->player->fov, x, y)
                                    ? fg_color
                                    : color_dark_gray);
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
                            color_black,
                            TCOD_BKGND_SET);
                    }
                }
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
                        if (tile->actor->faction == world->player->faction)
                        {
                            TCOD_console_printf_ex(
                                console,
                                view_rect.width / 2,
                                view_rect.height - 2,
                                TCOD_BKGND_NONE,
                                TCOD_CENTER,
                                "Lv.%d %s (%d/%d HP)",
                                tile->actor->level,
                                tile->actor->name,
                                tile->actor->hit_points,
                                actor_calc_max_hit_points(tile->actor));
                        }
                        else
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
                        }

                        goto done;
                    }

                    if (tile->corpse)
                    {
                        TCOD_console_printf_ex(
                            console,
                            view_rect.width / 2,
                            view_rect.height - 2,
                            TCOD_BKGND_NONE,
                            TCOD_CENTER,
                            "Lv.%d %s (dead)",
                            tile->corpse->level,
                            tile->corpse->name);

                        goto done;
                    }

                    if (tile->item)
                    {
                        TCOD_console_printf_ex(
                            console,
                            view_rect.width / 2,
                            view_rect.height - 2,
                            TCOD_BKGND_NONE,
                            TCOD_CENTER,
                            item_database[tile->item->type].name);

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
                const int max_hit_points = actor_calc_max_hit_points(world->player);
                const TCOD_ColorRGB fg =
                    (float)world->player->hit_points / max_hit_points > 0.5f
                        ? TCOD_color_lerp(color_yellow, color_green, world->player->hit_points / (max_hit_points * 0.5f))
                        : TCOD_color_lerp(color_red, color_yellow, world->player->hit_points / (max_hit_points * 0.5f));

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
                    max_hit_points);
            }

            if (class_database[world->player->class].spellcasting_ability != ABILITY_NONE)
            {
                const int max_mana = actor_calc_max_mana(world->player);
                const TCOD_ColorRGB fg = TCOD_color_lerp(color_gray, color_azure, (float)world->player->mana / max_mana);

                console_print(
                    status_rect.console,
                    1,
                    y++,
                    &fg,
                    NULL,
                    TCOD_BKGND_NONE,
                    TCOD_LEFT,
                    "MP: %d/%d",
                    world->player->mana,
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
                "CL: %d%s",
                world->player->level,
                world->player->ability_points > 0 ? "*" : "");

            console_print(
                status_rect.console,
                1,
                y++,
                &color_white,
                NULL,
                TCOD_BKGND_NONE,
                TCOD_LEFT,
                "DL: %d",
                world->player->floor + 1);

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
                1, 1);
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
                1, 1);
        }

        TCOD_console_blit(
            hud_rect.console,
            0, 0, hud_rect.width, hud_rect.height,
            console,
            hud_rect.x, hud_rect.y,
            1, 1);
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
                "%d/%d",
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

            TCOD_console_printf(
                panel_rect.console,
                1,
                y - current_panel_status->scroll,
                "Weight");
            TCOD_console_printf_ex(
                panel_rect.console,
                panel_rect.width - 2,
                y - current_panel_status->scroll,
                TCOD_BKGND_NONE,
                TCOD_RIGHT,
                "%.1f/%.1f",
                actor_calc_carry_weight(world->player),
                actor_calc_max_carry_weight(world->player));
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

            for (enum ability ability = ABILITY_NONE + 1; ability < NUM_ABILITIES; ability++)
            {
                if (character_action == CHARACTER_ACTION_ABILITY_ADD_POINT)
                {
                    console_print(
                        panel_rect.console,
                        1,
                        y - current_panel_status->scroll,
                        &color_white,
                        NULL,
                        TCOD_BKGND_NONE,
                        TCOD_LEFT,
                        "%c) %s",
                        ability - 1 + SDLK_a,
                        ability_database[ability].name);
                }
                else
                {
                    console_print(
                        panel_rect.console,
                        1,
                        y - current_panel_status->scroll,
                        &color_white,
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
                    &color_white,
                    NULL,
                    TCOD_BKGND_NONE,
                    TCOD_RIGHT,
                    "%d (%d)",
                    actor_calc_ability_score(world->player, ability),
                    actor_calc_ability_modifer(world->player, ability));
                y++;
            }

            y++;

            for (enum equip_slot equip_slot = EQUIP_SLOT_NONE + 1; equip_slot < NUM_EQUIP_SLOTS; equip_slot++)
            {
                const struct equip_slot_data equip_slot_data = equip_slot_database[equip_slot];

                if (character_action == CHARACTER_ACTION_EQUIPMENT_EXAMINE ||
                    character_action == CHARACTER_ACTION_EQUIPMENT_UNEQUIP)
                {
                    console_print(
                        panel_rect.console,
                        1,
                        y - current_panel_status->scroll,
                        &color_white,
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
                        &color_white,
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
                        &color_white,
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
                "%d/%d",
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
                "%d/%d",
                world->player->mana,
                actor_calc_max_mana(world->player));
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

            const struct item *const armor = world->player->equipment[EQUIP_SLOT_ARMOR];

            if (armor)
            {
                TCOD_console_printf(
                    panel_rect.console,
                    1,
                    y - current_panel_status->scroll,
                    "Max Dexterity Bonus");
                TCOD_console_printf_ex(
                    panel_rect.console,
                    panel_rect.width - 2,
                    y - current_panel_status->scroll,
                    TCOD_BKGND_NONE,
                    TCOD_RIGHT,
                    "%d",
                    base_item_database[item_database[armor->type].type].max_dexterity_bonus);
            }
            else
            {
                TCOD_console_printf(
                    panel_rect.console,
                    1,
                    y - current_panel_status->scroll,
                    "Max Dexterity Bonus");
                TCOD_console_printf_ex(
                    panel_rect.console,
                    panel_rect.width - 2,
                    y - current_panel_status->scroll,
                    TCOD_BKGND_NONE,
                    TCOD_RIGHT,
                    "None");
            }
            y++;

            TCOD_console_printf(
                panel_rect.console,
                1,
                y - current_panel_status->scroll,
                "Arcane Spell Failure");
            TCOD_console_printf_ex(
                panel_rect.console,
                panel_rect.width - 2,
                y - current_panel_status->scroll,
                TCOD_BKGND_NONE,
                TCOD_RIGHT,
                "%d%%",
                (int)(actor_calc_arcane_spell_failure(world->player) * 100));
            y++;

            TCOD_console_printf(
                panel_rect.console,
                1,
                y - current_panel_status->scroll,
                "Base Attack Bonus");
            const int base_attack_bonus = actor_calc_base_attack_bonus(world->player);
            char base_attack_bonus_string[20] = {0};
            snprintf(base_attack_bonus_string, sizeof(base_attack_bonus_string), "+%d", base_attack_bonus);
            const int attacks_per_round = actor_calc_attacks_per_round(world->player);
            for (int i = 1; i < attacks_per_round; i++)
            {
                char extra_attack_string[10] = {0};
                snprintf(extra_attack_string, sizeof(extra_attack_string), "/+%d", base_attack_bonus - i * 5);
                strcat(base_attack_bonus_string, extra_attack_string);
            }
            TCOD_console_printf_ex(
                panel_rect.console,
                panel_rect.width - 2,
                y - current_panel_status->scroll,
                TCOD_BKGND_NONE,
                TCOD_RIGHT,
                base_attack_bonus_string);
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

            if (actor_calc_carry_weight(world->player) <= actor_calc_max_carry_weight(world->player))
            {
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
                    "%.2f",
                    actor_calc_speed(world->player));
            }
            else
            {
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
                    "%.2f (overburdened)",
                    actor_calc_speed(world->player));
            }
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
            int y = 1;
            for (size_t item_index = 0; item_index < world->player->items->size; item_index++)
            {
                const struct item *const item = list_get(world->player->items, item_index);
                const struct item_data item_data = item_database[item->type];

                if (inventory_action == INVENTORY_ACTION_NONE)
                {
                    console_print(
                        panel_rect.console,
                        1,
                        y - current_panel_status->scroll,
                        &item_data.color,
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
                        &item_data.color,
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
            int y = 1;

            bool spells[NUM_SPELL_TYPES] = {false};
            actor_calc_spells(world->player, &spells);
            for (enum spell_type spell_type = SPELL_TYPE_NONE + 1; spell_type < NUM_SPELL_TYPES; spell_type++)
            {
                if (spells[spell_type])
                {
                    const struct spell_data *const spell_data = &spell_database[spell_type];

                    if (spellbook_action == SPELLBOOK_ACTION_NONE)
                    {
                        console_print(
                            panel_rect.console,
                            1,
                            y - current_panel_status->scroll,
                            &color_white,
                            NULL,
                            TCOD_BKGND_NONE,
                            TCOD_LEFT,
                            world->player->readied_spell == spell_type ? "%s (readied)" : "%s",
                            spell_data->name);
                    }
                    else
                    {
                        console_print(
                            panel_rect.console,
                            1,
                            y - current_panel_status->scroll,
                            &color_white,
                            NULL,
                            TCOD_BKGND_NONE,
                            TCOD_LEFT,
                            world->player->readied_spell == spell_type ? "%c) %s (readied)" : "%c) %s",
                            y - 1 + SDLK_a - current_panel_status->scroll,
                            spell_data->name);
                    }

                    console_print(
                        panel_rect.console,
                        panel_rect.width - 2,
                        y - current_panel_status->scroll,
                        &color_white,
                        NULL,
                        TCOD_BKGND_NONE,
                        TCOD_RIGHT,
                        "%d",
                        actor_calc_spell_mana_cost(world->player, spell_type));

                    y++;
                }
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
            1, 1);
    }

    if (!world->doomed && !world_can_player_take_turn())
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
