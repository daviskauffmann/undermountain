#include "scene_game.h"

#include "config.h"
#include "ability.h"
#include "actor.h"
#include "base_item.h"
#include "class.h"
#include "corpse.h"
#include "direction.h"
#include "event.h"
#include "explosion.h"
#include "item.h"
#include "light.h"
#include "list.h"
#include "map.h"
#include "message.h"
#include "object.h"
#include "projectile.h"
#include "race.h"
#include "saving_throw.h"
#include "spell.h"
#include "spell_range.h"
#include "surface.h"
#include "tile.h"
#include "world.h"
#include "scene_menu.h"
#include "scene.h"
#include "terminal.h"
#include "ui.h"
#include <curses.h>
#include <errno.h>
#include <libtcod.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MESSAGE_LOG_HEIGHT 5
#define STATUS_WIDTH 20

static int view_x = 0;
static int view_y = 0;
static int view_width = 0;
static int view_height = 0;

enum overlay
{
    OVERLAY_NONE,
    OVERLAY_INVENTORY,
    OVERLAY_CHARACTER,
    OVERLAY_SPELLBOOK,
    OVERLAY_HELP,
    OVERLAY_EXAMINE,
};

static enum overlay active_overlay = OVERLAY_NONE;
static int overlay_scroll = 0;
static bool hud_visible = true;
static float lighting_time;

enum input_mode
{
    INPUT_NORMAL,
    INPUT_DIRECTION,
    INPUT_SELECTION,
    INPUT_TARGET,
};

enum action
{
    ACTION_NONE,
    ACTION_OPEN,
    ACTION_CLOSE,
    ACTION_DIP,
    ACTION_SWING,
    ACTION_DROP,
    ACTION_EQUIP,
    ACTION_UNEQUIP,
    ACTION_QUAFF,
    ACTION_READ,
    ACTION_LEARN,
    ACTION_EXAMINE_ITEM,
    ACTION_EXAMINE_EQUIPMENT,
    ACTION_SPEND_POINT,
    ACTION_READY,
    ACTION_LOOK,
    ACTION_EXAMINE,
    ACTION_SHOOT,
    ACTION_CAST,
};

static enum input_mode input_mode = INPUT_NORMAL;
static enum action current_action = ACTION_NONE;
static int selected_index;
static int target_x, target_y;
static struct item *targeting_item;
static enum item_type examined_item = ITEM_TYPE_NONE;
static struct actor *action_actor;
static char overlay_notice[512];
static bool collecting_feedback;

static void show_notice(const struct message_data *const data)
{
    char *const text = message_format(data);
    if (!text)
    {
        return;
    }

    if (active_overlay == OVERLAY_INVENTORY || active_overlay == OVERLAY_CHARACTER ||
        active_overlay == OVERLAY_SPELLBOOK)
    {
        snprintf(overlay_notice, sizeof(overlay_notice), "%s", text);
    }
    else
    {
        message_add(text, data->type);
    }

    free(text);
}

static void receive_action_feedback(const struct game_event *const event, void *const context)
{
    (void)context;
    if (!collecting_feedback || !world_event_is_visible(event) ||
        (active_overlay != OVERLAY_INVENTORY && active_overlay != OVERLAY_CHARACTER &&
         active_overlay != OVERLAY_SPELLBOOK))
    {
        return;
    }

    struct message_data data;
    if (!message_from_event(event, &data))
    {
        return;
    }

    show_notice(&data);
}

static struct event_listener feedback_listener = {
    .receive = receive_action_feedback,
};

static const char *const action_names[] = {
    [ACTION_NONE] = "Game",
    [ACTION_OPEN] = "Open Door",
    [ACTION_CLOSE] = "Close Door",
    [ACTION_DIP] = "Dip",
    [ACTION_SWING] = "Attack",
    [ACTION_DROP] = "Drop",
    [ACTION_EQUIP] = "Equip",
    [ACTION_UNEQUIP] = "Unequip",
    [ACTION_QUAFF] = "Quaff",
    [ACTION_READ] = "Read",
    [ACTION_LEARN] = "Learn",
    [ACTION_EXAMINE_ITEM] = "Examine Item",
    [ACTION_EXAMINE_EQUIPMENT] = "Examine Equipment",
    [ACTION_SPEND_POINT] = "Spend Ability Point",
    [ACTION_READY] = "Ready Spell",
    [ACTION_LOOK] = "Look",
    [ACTION_EXAMINE] = "Examine",
    [ACTION_SHOOT] = "Shoot",
    [ACTION_CAST] = "Cast",
};

static void cancel_action(void)
{
    input_mode = INPUT_NORMAL;
    current_action = ACTION_NONE;
    targeting_item = NULL;
    action_actor = NULL;
}

static void begin_action(const enum input_mode mode, const enum action action)
{
    cancel_action();
    overlay_notice[0] = '\0';
    input_mode = mode;
    current_action = action;
    action_actor = world->player;
}

static void open_overlay(const enum overlay overlay)
{
    cancel_action();
    overlay_notice[0] = '\0';
    if (active_overlay == overlay)
    {
        active_overlay = OVERLAY_NONE;
    }
    else
    {
        active_overlay = overlay;
        overlay_scroll = 0;
    }
}

static void init(const struct scene *const previous_scene)
{
    (void)previous_scene;
    cancel_action();
    overlay_notice[0] = '\0';
    active_overlay = OVERLAY_NONE;
    overlay_scroll = 0;
    hud_visible = true;
    lighting_time = 0;
    collecting_feedback = false;
    event_subscribe(&feedback_listener);

    int max_y, max_x;
    getmaxyx(stdscr, max_y, max_x);

    view_width = max_x;
    view_height = max_y - MESSAGE_LOG_HEIGHT;
}

static void uninit(void)
{
    if (world)
    {
        event_unsubscribe(&feedback_listener);
    }
    if (world && !world->doomed)
    {
        FILE *const file = fopen(SAVE_PATH, "wb");
        if (file)
        {
            world_save(file);
            bool failed = ferror(file) != 0;
            if (fclose(file) != 0)
            {
                failed = true;
            }
            if (failed)
            {
                fprintf(stderr, "Could not write save: %s\n", SAVE_PATH);
            }
        }
        else
        {
            fprintf(stderr, "Could not open save %s: %s\n", SAVE_PATH, strerror(errno));
        }
    }

    if (world)
    {
        world_uninit();
    }
    cancel_action();
}

static bool try_move_player(const enum direction direction)
{
    if (!world || !world->player)
    {
        return false;
    }

    int x, y;
    get_neighbor_by_direction(world->player->x, world->player->y, direction, &x, &y);

    return actor_move(world->player, x, y);
}

static bool key_direction(const int ch, enum direction *const direction)
{
    switch (ch)
    {
    case KEY_UP:
    case KEY_A2:
    case '8':
    case 'k':
    case 'K':
        *direction = DIRECTION_N;
        return true;
    case KEY_DOWN:
    case KEY_C2:
    case '2':
    case 'j':
    case 'J':
        *direction = DIRECTION_S;
        return true;
    case KEY_LEFT:
    case KEY_B1:
    case '4':
    case 'h':
    case 'H':
        *direction = DIRECTION_W;
        return true;
    case KEY_RIGHT:
    case KEY_B3:
    case '6':
    case 'l':
    case 'L':
        *direction = DIRECTION_E;
        return true;
    case KEY_A1:
    case '7':
    case 'y':
    case 'Y':
        *direction = DIRECTION_NW;
        return true;
    case KEY_A3:
    case '9':
    case 'u':
    case 'U':
        *direction = DIRECTION_NE;
        return true;
    case KEY_C1:
    case '1':
    case 'b':
    case 'B':
        *direction = DIRECTION_SW;
        return true;
    case KEY_C3:
    case '3':
    case 'n':
    case 'N':
        *direction = DIRECTION_SE;
        return true;
    default:
        return false;
    }
}

static bool key_attack_direction(const int ch, enum direction *const direction)
{
    switch (ch)
    {
    case CTL_UP:
    case CTL_PAD8:
        *direction = DIRECTION_N;
        return true;
    case CTL_DOWN:
    case CTL_PAD2:
        *direction = DIRECTION_S;
        return true;
    case CTL_LEFT:
    case CTL_PAD4:
        *direction = DIRECTION_W;
        return true;
    case CTL_RIGHT:
    case CTL_PAD6:
        *direction = DIRECTION_E;
        return true;
    case CTL_PAD7:
        *direction = DIRECTION_NW;
        return true;
    case CTL_PAD9:
        *direction = DIRECTION_NE;
        return true;
    case CTL_PAD1:
        *direction = DIRECTION_SW;
        return true;
    case CTL_PAD3:
        *direction = DIRECTION_SE;
        return true;
    default:
        return false;
    }
}

static bool player_swing(const enum direction direction)
{
    struct actor *player = world->player;
    int x, y;
    get_neighbor_by_direction(player->x, player->y, direction, &x, &y);
    if (!map_is_inside(x, y))
    {
        return false;
    }
    if (actor_has_ranged_weapon(player) && actor_shoot(player, x, y))
    {
        return true;
    }

    const struct tile *tile = &world->maps[player->floor].tiles[x][y];
    if (tile->actor && tile->actor != player)
    {
        return actor_attack(player, tile->actor, false, NULL);
    }
    if (tile->object)
    {
        return actor_bash(player, tile->object);
    }
    event_emit(&(struct game_event){
        .type = EVENT_ATTACK_SWUNG,
        .floor = player->floor,
        .x = player->x,
        .y = player->y,
        .actor = player->name,
    });
    return false;
}

static void handle_direction(const int ch)
{
    enum direction direction;
    if (!key_direction(ch, &direction) && !key_attack_direction(ch, &direction))
    {
        return;
    }
    if (!world_can_player_take_turn())
    {
        return;
    }

    struct actor *player = world->player;
    int x, y;
    get_neighbor_by_direction(player->x, player->y, direction, &x, &y);
    bool took_turn = false;
    switch (current_action)
    {
    case ACTION_OPEN:
        took_turn = actor_open_door(player, x, y);
        break;
    case ACTION_CLOSE:
        took_turn = actor_close_door(player, x, y);
        break;
    case ACTION_DIP:
        took_turn = actor_dip(player, x, y);
        break;
    case ACTION_SWING:
        took_turn = player_swing(direction);
        break;
    default:
        break;
    }
    player->took_turn = took_turn;
    cancel_action();
}

static void begin_target(const enum action action, const bool nearest_enemy)
{
    begin_action(INPUT_TARGET, action);
    active_overlay = OVERLAY_NONE;
    const struct actor *target = nearest_enemy ? actor_find_nearest_enemy(world->player) : NULL;
    if (!target)
    {
        target = world->player;
    }
    target_x = target->x;
    target_y = target->y;
}

static void handle_target(const int ch)
{
    enum direction direction;
    if (key_direction(ch, &direction) || key_attack_direction(ch, &direction))
    {
        int x, y;
        get_neighbor_by_direction(target_x, target_y, direction, &x, &y);
        if (map_is_inside(x, y))
        {
            target_x = x;
            target_y = y;
        }
        return;
    }
    const bool confirm = ch == '\n' || ch == '\r' || ch == KEY_ENTER ||
                   (current_action == ACTION_SHOOT && ch == 'f') ||
                   (current_action == ACTION_CAST && ch == 'z') ||
                   (current_action == ACTION_READ && ch == 'r') ||
                   (current_action == ACTION_EXAMINE && ch == 'x');
    if (!confirm || current_action == ACTION_LOOK)
    {
        return;
    }

    if (current_action == ACTION_EXAMINE)
    {
        cancel_action();
        examined_item = ITEM_TYPE_NONE;
        active_overlay = OVERLAY_EXAMINE;
        overlay_scroll = 0;
        return;
    }
    if (!world_can_player_take_turn())
    {
        return;
    }
    switch (current_action)
    {
    case ACTION_SHOOT:
        world->player->took_turn = actor_shoot(world->player, target_x, target_y);
        break;
    case ACTION_CAST:
        world->player->took_turn = actor_cast(world->player, world->player->readied_spell, target_x, target_y, true);
        break;
    case ACTION_READ:
        world->player->took_turn = actor_read(world->player, targeting_item, target_x, target_y);
        break;
    default:
        break;
    }
    cancel_action();
}

static int selection_count(void)
{
    switch (current_action)
    {
    case ACTION_UNEQUIP:
    case ACTION_EXAMINE_EQUIPMENT:
        return NUM_EQUIP_SLOTS - 1;
    case ACTION_SPEND_POINT:
        return NUM_ABILITIES - 1;
    case ACTION_READY: {
        int count = 0;
        const struct actor_spells spells = actor_get_spells(world->player);
        for (enum spell_type spell = SPELL_TYPE_NONE + 1; spell < NUM_SPELL_TYPES; spell++)
        {
            if (spells.has[spell])
            {
                count++;
            }
        }
        return count;
    }
    default:
        return (int)world->player->items->size;
    }
}

static void begin_selection(const enum action action, const enum overlay overlay)
{
    begin_action(INPUT_SELECTION, action);
    active_overlay = overlay;
    overlay_scroll = 0;
    selected_index = 0;
    if (selection_count() == 0 || (action == ACTION_SPEND_POINT && world->player->ability_points <= 0))
    {
        show_notice(&(struct message_data){
            .type = MESSAGE_NOTHING_TO_SELECT,
            .action = action_names[action],
        });
        cancel_action();
    }
}

static void examine_item(const struct item *const item)
{
    if (!item)
    {
        show_notice(&(struct message_data){
            .type = MESSAGE_NO_EQUIPMENT,
        });
        return;
    }
    examined_item = item->type;
    active_overlay = OVERLAY_EXAMINE;
    overlay_scroll = 0;
}

static void select_action(int index)
{
    struct actor *player = world->player;
    const enum action action = current_action;
    if (!world_can_player_take_turn() && action != ACTION_EXAMINE_ITEM &&
        action != ACTION_EXAMINE_EQUIPMENT)
    {
        return;
    }

    overlay_notice[0] = '\0';
    if (action == ACTION_SPEND_POINT)
    {
        if (player->ability_points > 0)
        {
            actor_spend_ability_point(player, (enum ability)(index + 1));
            event_emit(&(struct game_event){
                .type = EVENT_ABILITY_INCREASED,
                .floor = -1,
                .x = 0,
                .y = 0,
                .ability = ability_database[index + 1].name,
                .amount = actor_get_ability_score(player, (enum ability)(index + 1)),
            });
        }
        if (player->ability_points == 0)
        {
            cancel_action();
        }
        return;
    }
    cancel_action();
    switch (action)
    {
    case ACTION_READY: {
        const struct actor_spells spells = actor_get_spells(player);
        for (enum spell_type spell = SPELL_TYPE_NONE + 1; spell < NUM_SPELL_TYPES; spell++)
        {
            if (spells.has[spell] && index-- == 0)
            {
                player->readied_spell = spell;
                event_emit(&(struct game_event){
                    .type = EVENT_SPELL_READIED,
                    .floor = player->floor,
                    .x = player->x,
                    .y = player->y,
                    .spell = spell_database[spell].name,
                });
                break;
            }
        }
        return;
    }
    case ACTION_UNEQUIP:
        player->took_turn = actor_unequip(player, (enum equip_slot)(index + 1));
        return;
    case ACTION_EXAMINE_EQUIPMENT:
        examine_item(player->equipment[index + 1]);
        return;
    default:
        break;
    }

    struct item *item = list_get(player->items, (size_t)index);
    switch (action)
    {
    case ACTION_DROP:
        player->took_turn = actor_drop(player, item);
        break;
    case ACTION_EQUIP:
        player->took_turn = actor_equip(player, item);
        break;
    case ACTION_QUAFF:
        player->took_turn = actor_quaff(player, item);
        break;
    case ACTION_LEARN:
        player->took_turn = actor_learn(player, item);
        break;
    case ACTION_EXAMINE_ITEM:
        examine_item(item);
        break;
    case ACTION_READ: {
        const struct item_data *data = &item_database[item->type];
        if (data->type == BASE_ITEM_TYPE_SCROLL && spell_database[data->spell_type].range == SPELL_RANGE_TOUCH)
        {
            begin_target(ACTION_READ, spell_database[data->spell_type].harmful);
            targeting_item = item;
        }
        else
        {
            player->took_turn = actor_read(player, item, player->x, player->y);
        }
        break;
    }
    default:
        break;
    }
}

static void handle_selection(int ch)
{
    const int count = selection_count();
    if (count == 0)
    {
        cancel_action();
        return;
    }
    if (ch >= 'A' && ch <= 'Z')
    {
        ch += 'a' - 'A';
    }
    if (ch >= 'a' && ch <= 'z')
    {
        const int index = ch - 'a';
        if (index < count)
        {
            select_action(index);
        }
        else
        {
            beep();
        }
        return;
    }
    const int page = getmaxy(stdscr) > 3 ? getmaxy(stdscr) - 3 : 1;
    switch (ch)
    {
    case KEY_UP:
        selected_index--;
        break;
    case KEY_DOWN:
        selected_index++;
        break;
    case KEY_PPAGE:
        selected_index -= page;
        break;
    case KEY_NPAGE:
        selected_index += page;
        break;
    case KEY_HOME:
        selected_index = 0;
        break;
    case KEY_END:
        selected_index = count - 1;
        break;
    case '\n':
    case '\r':
    case KEY_ENTER:
        select_action(selected_index);
        return;
    default:
        return;
    }
    if (selected_index < 0)
    {
        selected_index = 0;
    }
    if (selected_index >= count)
    {
        selected_index = count - 1;
    }
}

static bool handle_overlay_key(const int ch)
{
    const int page = getmaxy(stdscr) > 3 ? getmaxy(stdscr) - 3 : 1;
    switch (ch)
    {
    case 'i':
    case 'I':
        open_overlay(OVERLAY_INVENTORY);
        return true;
    case 'c':
    case 'C':
        open_overlay(OVERLAY_CHARACTER);
        return true;
    case 'v':
    case 'V':
        open_overlay(OVERLAY_SPELLBOOK);
        return true;
    case '?':
        open_overlay(OVERLAY_HELP);
        return true;
    default:
        break;
    }
    if (active_overlay == OVERLAY_NONE)
    {
        return false;
    }
    switch (ch)
    {
    case KEY_UP:
    case 'k':
    case 'K':
        overlay_scroll--;
        break;
    case KEY_DOWN:
    case 'j':
    case 'J':
        overlay_scroll++;
        break;
    case KEY_PPAGE:
        overlay_scroll -= page;
        break;
    case KEY_NPAGE:
        overlay_scroll += page;
        break;
    case KEY_HOME:
        overlay_scroll = 0;
        break;
    default:
        return false;
    }
    if (overlay_scroll < 0)
    {
        overlay_scroll = 0;
    }
    return true;
}

static struct scene *input(void)
{
    const int ch = ui_read_key();
    if (ch == ERR)
    {
        return &scene_game;
    }
    if (ch == 27)
    {
        if (input_mode != INPUT_NORMAL)
        {
            cancel_action();
        }
        else if (active_overlay != OVERLAY_NONE)
        {
            active_overlay = OVERLAY_NONE;
            overlay_notice[0] = '\0';
        }
        else
        {
            return &scene_menu;
        }
        return &scene_game;
    }
    switch (input_mode)
    {
    case INPUT_DIRECTION:
        handle_direction(ch);
        return &scene_game;
    case INPUT_SELECTION:
        handle_selection(ch);
        return &scene_game;
    case INPUT_TARGET:
        handle_target(ch);
        return &scene_game;
    default:
        break;
    }
    if (handle_overlay_key(ch))
    {
        return &scene_game;
    }

    if (active_overlay == OVERLAY_INVENTORY)
    {
        switch (ch)
        {
        case 'd':
            begin_selection(ACTION_DROP, OVERLAY_INVENTORY);
            break;
        case 'e':
            begin_selection(ACTION_EQUIP, OVERLAY_INVENTORY);
            break;
        case 'U':
            begin_selection(ACTION_UNEQUIP, OVERLAY_INVENTORY);
            break;
        case 'q':
            begin_selection(ACTION_QUAFF, OVERLAY_INVENTORY);
            break;
        case 'r':
            begin_selection(ACTION_READ, OVERLAY_INVENTORY);
            break;
        case 'L':
            begin_selection(ACTION_LEARN, OVERLAY_INVENTORY);
            break;
        case 'X':
            begin_selection(ACTION_EXAMINE_ITEM, OVERLAY_INVENTORY);
            break;
        case 24:
            begin_selection(ACTION_EXAMINE_EQUIPMENT, OVERLAY_INVENTORY);
            break;
        default:
            break;
        }
        return &scene_game;
    }
    if (active_overlay == OVERLAY_CHARACTER)
    {
        if (ch == 'a' || ch == 'A')
        {
            begin_selection(ACTION_SPEND_POINT, OVERLAY_CHARACTER);
        }
        return &scene_game;
    }
    if (active_overlay == OVERLAY_SPELLBOOK)
    {
        if (ch == 'Z')
        {
            begin_selection(ACTION_READY, OVERLAY_SPELLBOOK);
        }
        return &scene_game;
    }
    if (active_overlay != OVERLAY_NONE)
    {
        return &scene_game;
    }

    if (ch == 'H')
    {
        hud_visible = !hud_visible;
        return &scene_game;
    }
    if (ch == 'w' || ch == 'x')
    {
        begin_target(ch == 'w' ? ACTION_LOOK : ACTION_EXAMINE, false);
        return &scene_game;
    }
    if (ch == 'X' || ch == 24)
    {
        begin_selection(ch == 'X' ? ACTION_EXAMINE_ITEM : ACTION_EXAMINE_EQUIPMENT, OVERLAY_INVENTORY);
        return &scene_game;
    }
    if (!world_can_player_take_turn())
    {
        return &scene_game;
    }

    struct actor *player = world->player;
    enum direction direction;
    if (key_attack_direction(ch, &direction))
    {
        player->took_turn = player_swing(direction);
        return &scene_game;
    }
    if (ch != 'L' && ch != 'U' && key_direction(ch, &direction))
    {
        player->took_turn = try_move_player(direction);
        return &scene_game;
    }
    switch (ch)
    {
    case '.':
    case '5':
    case KEY_B2:
        player->took_turn = true;
        break;
    case 'g':
    case ',':
        player->took_turn = actor_grab(player, player->x, player->y);
        break;
    case '<':
        player->took_turn = actor_ascend(player);
        break;
    case '>':
        player->took_turn = actor_descend(player);
        break;
    case 'R':
        player->took_turn = actor_rest(player);
        break;
    case 'o':
        begin_action(INPUT_DIRECTION, ACTION_OPEN);
        break;
    case 'O':
        begin_action(INPUT_DIRECTION, ACTION_CLOSE);
        break;
    case 'D':
        begin_action(INPUT_DIRECTION, ACTION_DIP);
        break;
    case 'F':
        begin_action(INPUT_DIRECTION, ACTION_SWING);
        break;
    case 'd':
        begin_selection(ACTION_DROP, OVERLAY_INVENTORY);
        break;
    case 'e':
        begin_selection(ACTION_EQUIP, OVERLAY_INVENTORY);
        break;
    case 'U':
        begin_selection(ACTION_UNEQUIP, OVERLAY_INVENTORY);
        break;
    case 'q':
        begin_selection(ACTION_QUAFF, OVERLAY_INVENTORY);
        break;
    case 'r':
        begin_selection(ACTION_READ, OVERLAY_INVENTORY);
        break;
    case 'L':
        begin_selection(ACTION_LEARN, OVERLAY_INVENTORY);
        break;
    case 'a':
    case 'A':
        begin_selection(ACTION_SPEND_POINT, OVERLAY_CHARACTER);
        break;
    case 'Z':
        begin_selection(ACTION_READY, OVERLAY_SPELLBOOK);
        break;
    case 'f':
        begin_target(ACTION_SHOOT, true);
        break;
    case 'z':
        if (player->readied_spell == SPELL_TYPE_NONE)
        {
            show_notice(&(struct message_data){
                .type = MESSAGE_NO_READIED_SPELL,
            });
        }
        else if (spell_database[player->readied_spell].range == SPELL_RANGE_PERSONAL)
        {
            player->took_turn = actor_cast(player, player->readied_spell, player->x, player->y, true);
        }
        else
        {
            begin_target(ACTION_CAST, spell_database[player->readied_spell].harmful);
        }
        break;
    case 't':
    case 'T': {
        const enum light_type light = ch == 't' ? LIGHT_TYPE_TORCH : LIGHT_TYPE_GLOW;
        player->light_type = player->light_type == light ? LIGHT_TYPE_PLAYER : light;
        player->took_turn = true;
        break;
    }
    case 11:
        actor_die(player, NULL);
        break;
    default:
        break;
    }
    return &scene_game;
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

static void draw_character_identity(int *const row, const int scroll, const int col, const int width, const struct actor *const actor, const int max_rows)
{
    char value[64];

    ui_panel_header(row, scroll, col, width, "Identity", max_rows);

    ui_panel_value(row, scroll, col, width, "Name", actor->name, max_rows);
    ui_panel_value(row, scroll, col, width, "Race", race_database[actor->race].name, max_rows);
    ui_panel_value(row, scroll, col, width, "Class", class_database[actor->class].name, max_rows);

    snprintf(value, sizeof(value), "%d", actor->level);
    ui_panel_value(row, scroll, col, width, "Level", value, max_rows);

    snprintf(value, sizeof(value), "%d / %d", actor->experience,
             actor_get_experience_for_level(actor->level + 1));
    ui_panel_value(row, scroll, col, width, "XP", value, max_rows);

    snprintf(value, sizeof(value), "%d gp", actor->gold);
    ui_panel_value(row, scroll, col, width, "Gold", value, max_rows);

    snprintf(value, sizeof(value), "%.1f / %.1f lb",
             actor_get_carry_weight(actor),
             actor_get_max_carry_weight(actor));
    ui_panel_value(row, scroll, col, width, "Load", value, max_rows);

    snprintf(value, sizeof(value), "%d%s", actor->ability_points, actor->ability_points > 0 ? " (unspent)" : "");
    ui_panel_value(row, scroll, col, width, "Ability Points", value, max_rows);
}

static void draw_character_abilities(int *const row, const int scroll, const int col, const int width, const struct actor *const actor, const int max_rows)
{
    char value[64];

    ui_panel_blank(row);
    ui_panel_header(row, scroll, col, width, "Abilities", max_rows);

    for (enum ability ability = ABILITY_NONE + 1; ability < NUM_ABILITIES; ability++)
    {
        char label[64];
        const bool selecting = current_action == ACTION_SPEND_POINT;
        const bool selected = selecting && selected_index == ability - 1;
        if (selecting)
        {
            snprintf(label, sizeof(label), "%c) %s", 'a' + ability - 1, ability_database[ability].name);
        }
        else
        {
            snprintf(label, sizeof(label), "%s", ability_database[ability].name);
        }
        snprintf(value, sizeof(value), "%d  (%+d)",
                 actor_get_ability_score(actor, ability),
                 actor_get_ability_modifer(actor, ability));
        if (selected && ui_panel_row_in_view(*row, scroll, max_rows))
        {
            attron(A_REVERSE | A_BOLD);
        }
        ui_panel_value(row, scroll, col, width, label, value, max_rows);
        if (selected && ui_panel_row_in_view(*row - 1, scroll, max_rows))
        {
            attroff(A_REVERSE | A_BOLD);
        }
    }
}

static void draw_character_combat(int *const row, const int scroll, const int col, const int width, const struct actor *const actor, const int max_rows)
{
    char value[64];

    ui_panel_header(row, scroll, col, width, "Combat", max_rows);

    snprintf(value, sizeof(value), "%d / %d",
             actor->hit_points, actor_get_max_hit_points(actor));
    ui_panel_value(row, scroll, col, width, "HP", value, max_rows);

    const int max_mana = actor_get_max_mana(actor);
    if (max_mana > 0)
    {
        snprintf(value, sizeof(value), "%d / %d", actor->mana, max_mana);
        ui_panel_value(row, scroll, col, width, "MP", value, max_rows);
    }

    snprintf(value, sizeof(value), "%d", actor_get_armor_class(actor));
    ui_panel_value(row, scroll, col, width, "Armor Class", value, max_rows);

    const struct item *const armor = actor->equipment[EQUIP_SLOT_ARMOR];
    if (armor)
    {
        snprintf(value, sizeof(value), "%d",
                 base_item_database[item_database[armor->type].type].max_dexterity_bonus);
    }
    else
    {
        snprintf(value, sizeof(value), "None");
    }
    ui_panel_value(row, scroll, col, width, "Max Dex", value, max_rows);

    snprintf(value, sizeof(value), "%d%%",
             (int)(actor_get_arcane_spell_failure(actor) * 100));
    ui_panel_value(row, scroll, col, width, "Spell Fail", value, max_rows);

    const int base_attack_bonus = actor_get_base_attack_bonus(actor);
    const int attacks_per_round = actor_get_attacks_per_round(actor);
    char attacks[32];
    snprintf(attacks, sizeof(attacks), "+%d", base_attack_bonus);
    for (int attack = 1; attack < attacks_per_round; attack++)
    {
        char extra[16];
        snprintf(extra, sizeof(extra), " / +%d", base_attack_bonus - attack * 5);
        strncat(attacks, extra, sizeof(attacks) - strlen(attacks) - 1);
    }
    ui_panel_value(row, scroll, col, width, "Base Attack", attacks, max_rows);

    snprintf(value, sizeof(value), "%+d", actor_get_attack_bonus(actor));
    ui_panel_value(row, scroll, col, width, "Attack", value, max_rows);

    const int threat = actor_get_threat_range(actor);
    if (threat == 20)
    {
        snprintf(value, sizeof(value), "%s+%d (x%d)",
                 actor_get_damage(actor),
                 actor_get_damage_bonus(actor),
                 actor_get_critical_multiplier(actor));
    }
    else
    {
        snprintf(value, sizeof(value), "%s+%d (%d-20/x%d)",
                 actor_get_damage(actor),
                 actor_get_damage_bonus(actor),
                 threat,
                 actor_get_critical_multiplier(actor));
    }
    ui_panel_value(row, scroll, col, width, "Damage", value, max_rows);

    const float speed = actor_get_speed(actor);
    const bool overburdened = actor_get_carry_weight(actor) > actor_get_max_carry_weight(actor);
    if (overburdened)
    {
        snprintf(value, sizeof(value), "%.2f (overburdened)", speed);
    }
    else
    {
        snprintf(value, sizeof(value), "%.2f", speed);
    }
    ui_panel_value(row, scroll, col, width, "Speed", value, max_rows);
}

static void draw_character_saves(int *const row, const int scroll, const int col, const int width, const struct actor *const actor, const int max_rows)
{
    char value[64];

    ui_panel_blank(row);
    ui_panel_header(row, scroll, col, width, "Saves", max_rows);

    for (enum saving_throw save = SAVING_THROW_NONE + 1; save < NUM_SAVING_THROWS; save++)
    {
        snprintf(value, sizeof(value), "%+d", actor_get_saving_throw(actor, save));
        ui_panel_value(row, scroll, col, width, saving_throw_database[save].name, value, max_rows);
    }
}

static void draw_equipment(int *const row, const int col, const int width, const int max_y)
{
    const bool selecting = current_action == ACTION_UNEQUIP || current_action == ACTION_EXAMINE_EQUIPMENT;
    ui_panel_header(row, overlay_scroll, col, width, "Equipment", max_y);
    for (enum equip_slot slot = EQUIP_SLOT_NONE + 1; slot < NUM_EQUIP_SLOTS; slot++)
    {
        const struct item *equipment = world->player->equipment[slot];
        char text[192];
        if (selecting)
        {
            snprintf(text, sizeof(text), "%c) %s: %s", 'a' + slot - 1,
                     equip_slot_database[slot].name, equipment ? item_database[equipment->type].name : "(empty)");
        }
        else
        {
            snprintf(text, sizeof(text), "%s: %s", equip_slot_database[slot].name,
                     equipment ? item_database[equipment->type].name : "(empty)");
        }
        ui_panel_item(row, overlay_scroll, col, width, text, selecting && selected_index == slot - 1, max_y);
    }
}

static void draw_carried_items(int *const row, const int col, const int width, const int max_y)
{
    const bool selecting = input_mode == INPUT_SELECTION &&
                     current_action != ACTION_UNEQUIP && current_action != ACTION_EXAMINE_EQUIPMENT;
    ui_panel_header(row, overlay_scroll, col, width, "Carried Items", max_y);
    if (world->player->items->size == 0)
    {
        ui_panel_item(row, overlay_scroll, col, width, "(nothing)", false, max_y);
        return;
    }
    for (size_t index = 0; index < world->player->items->size; index++)
    {
        const struct item *item = list_get(world->player->items, index);
        char label[16] = "";
        char text[192];
        if (selecting)
        {
            if (index < 26)
            {
                snprintf(label, sizeof(label), "%c) ", 'a' + (int)index);
            }
            else
            {
                snprintf(label, sizeof(label), "%zu) ", index + 1);
            }
        }
        if (item->stack > 1)
        {
            snprintf(text, sizeof(text), "%s%s (%d)", label, item_database[item->type].name, item->stack);
        }
        else
        {
            snprintf(text, sizeof(text), "%s%s", label, item_database[item->type].name);
        }
        ui_panel_item(row, overlay_scroll, col, width, text, selecting && selected_index == (int)index, max_y);
    }
}

static void draw_inventory_overlay(const int max_y, const int max_x)
{
    const bool selecting = input_mode == INPUT_SELECTION;
    ui_panel_frame(selecting ? action_names[current_action] : "Inventory",
                   selecting ? "[Up/Down] select  [a-z/Enter] confirm  [PgUp/PgDn] page  [ESC] cancel" : "[↑↓/jk] scroll  [PgUp/PgDn] page  [ESC] close  [?] help");

    const bool columns = max_x >= 72;
    const int divider = max_x / 2;
    const int equipment_rows = NUM_EQUIP_SLOTS;
    const int item_rows = 1 + (world->player->items->size ? (int)world->player->items->size : 1);
    const int total_rows = columns ? (equipment_rows > item_rows ? equipment_rows : item_rows) : equipment_rows + 1 + item_rows;
    if (selecting)
    {
        const bool equipment = current_action == ACTION_UNEQUIP || current_action == ACTION_EXAMINE_EQUIPMENT;
        const int selection_row = selected_index + 1 + (!equipment && !columns ? equipment_rows + 1 : 0);
        overlay_scroll = ui_panel_show_row(overlay_scroll, selection_row, max_y);
    }
    overlay_scroll = ui_panel_scroll(overlay_scroll, total_rows, max_y);
    int row = 0;
    if (columns)
    {
        ui_panel_divider(divider, max_y);
        draw_equipment(&row, 2, divider - 3, max_y);
        row = 0;
        draw_carried_items(&row, divider + 2, max_x - divider - 3, max_y);
    }
    else
    {
        draw_equipment(&row, 2, max_x - 4, max_y);
        ui_panel_blank(&row);
        draw_carried_items(&row, 2, max_x - 4, max_y);
    }
    ui_panel_scrollbar(total_rows, max_y, max_x, overlay_scroll);
}

static void draw_character_sheet_overlay(const int max_y, const int max_x)
{
    ui_panel_frame(current_action == ACTION_SPEND_POINT ? "Spend Ability Point" : "Character Sheet",
                   current_action == ACTION_SPEND_POINT ? "[Up/Down] select  [a-f/Enter] spend  [ESC] cancel" : "[↑↓/jk] scroll  [PgUp/PgDn] page  [ESC] close  [a] spend points");

    const bool columns = max_x >= 72;
    const int divider = max_x / 2;
    const int left_width = columns ? divider - 3 : max_x - 4;
    const int right_width = columns ? max_x - divider - 3 : max_x - 4;
    const int right_column = columns ? divider + 2 : 2;

    // Measure with no visible rows so both columns share one scroll range.
    int left_rows = 0, right_rows = 0;
    draw_character_identity(&left_rows, overlay_scroll, 2, left_width, world->player, 0);
    const int ability_row = left_rows + 2;
    draw_character_abilities(&left_rows, overlay_scroll, 2, left_width, world->player, 0);
    draw_character_combat(&right_rows, overlay_scroll, right_column, right_width, world->player, 0);
    draw_character_saves(&right_rows, overlay_scroll, right_column, right_width, world->player, 0);
    const int total_rows = columns ? (left_rows > right_rows ? left_rows : right_rows) : left_rows + 1 + right_rows;
    if (current_action == ACTION_SPEND_POINT)
    {
        overlay_scroll = ui_panel_show_row(overlay_scroll, ability_row + selected_index, max_y);
    }
    overlay_scroll = ui_panel_scroll(overlay_scroll, total_rows, max_y);

    int row = 0;
    draw_character_identity(&row, overlay_scroll, 2, left_width, world->player, max_y);
    draw_character_abilities(&row, overlay_scroll, 2, left_width, world->player, max_y);
    if (columns)
    {
        ui_panel_divider(divider, max_y);
        row = 0;
    }
    else
    {
        ui_panel_blank(&row);
    }
    draw_character_combat(&row, overlay_scroll, right_column, right_width, world->player, max_y);
    draw_character_saves(&row, overlay_scroll, right_column, right_width, world->player, max_y);
    ui_panel_scrollbar(total_rows, max_y, max_x, overlay_scroll);
}

static void draw_spellbook_overlay(const int max_y, const int max_x)
{
    const bool selecting = current_action == ACTION_READY;
    ui_panel_frame(selecting ? "Ready Spell" : "Spellbook",
                   selecting ? "[Up/Down] select  [a-z/Enter] ready  [ESC] cancel" : "[↑↓/jk] scroll  [PgUp/PgDn] page  [ESC] close  [Z] ready");

    const int col = 2;
    const int content_width = max_x - 4;
    const int name_width = content_width > 18 ? content_width - 12 : content_width / 2;
    const struct actor_spells player_spells = actor_get_spells(world->player);
    int count = 0;
    for (enum spell_type spell = SPELL_TYPE_NONE + 1; spell < NUM_SPELL_TYPES; spell++)
    {
        if (player_spells.has[spell])
        {
            count++;
        }
    }
    const int total_rows = 1 + (count ? count : 1);
    if (selecting)
    {
        overlay_scroll = ui_panel_show_row(overlay_scroll, selected_index + 1, max_y);
    }
    overlay_scroll = ui_panel_scroll(overlay_scroll, total_rows, max_y);

    char text[192];
    snprintf(text, sizeof(text), "%-*s Lvl  MP", name_width, "Spell (* readied)");
    int row = 0;
    ui_panel_header(&row, overlay_scroll, col, content_width, text, max_y);
    int index = 0;
    for (enum spell_type spell = SPELL_TYPE_NONE + 1; spell < NUM_SPELL_TYPES; spell++)
    {
        if (player_spells.has[spell])
        {
            char name[128];
            const struct spell_data *spell_data = &spell_database[spell];
            if (selecting)
            {
                snprintf(name, sizeof(name), "%c) %s%s", 'a' + index, spell_data->name, world->player->readied_spell == spell ? " *" : "");
            }
            else
            {
                snprintf(name, sizeof(name), "%s%s", spell_data->name, world->player->readied_spell == spell ? " *" : "");
            }
            snprintf(text, sizeof(text), "%-*.*s %3d %3d", name_width, name_width, name,
                     spell_data->level, actor_get_spell_mana_cost(world->player, spell));
            ui_panel_item(&row, overlay_scroll, col, content_width, text, selecting && selected_index == index, max_y);
            index++;
        }
    }
    if (!count)
    {
        ui_panel_item(&row, overlay_scroll, col, content_width, "(no spells known)", false, max_y);
    }
    ui_panel_scrollbar(total_rows, max_y, max_x, overlay_scroll);
}

static void describe_target(char *const text, const size_t size)
{
    const struct actor *player = world->player;
    const struct tile *tile = &world->maps[player->floor].tiles[target_x][target_y];
    if (!tile->explored)
    {
        snprintf(text, size, "Unknown");
    }
    else if (!TCOD_map_is_in_fov(player->fov, target_x, target_y))
    {
        snprintf(text, size, "%s (hidden)", tile_database[tile->type].name);
    }
    else if (tile->actor)
    {
        if (tile->actor->faction == player->faction)
        {
            snprintf(text, size, "Lv.%d %s (%d/%d HP)", tile->actor->level, tile->actor->name,
                     tile->actor->hit_points, actor_get_max_hit_points(tile->actor));
        }
        else
        {
            snprintf(text, size, "Lv.%d %s", tile->actor->level, tile->actor->name);
        }
    }
    else if (tile->object && (tile->object->type != OBJECT_TYPE_TRAP ||
                              tile->object->trap_detection_state == OBJECT_TRAP_DETECTION_STATE_DETECTED))
    {
        snprintf(text, size, "%s", object_database[tile->object->type].name);
    }
    else if (tile->item)
    {
        snprintf(text, size, "%s (%d)", item_database[tile->item->type].name, tile->item->stack);
    }
    else if (tile->corpse)
    {
        snprintf(text, size, "Lv.%d %s (dead)", tile->corpse->level, tile->corpse->name);
    }
    else if (tile->surface)
    {
        snprintf(text, size, "%s", surface_database[tile->surface->type].name);
    }
    else
    {
        snprintf(text, size, "%s", tile_database[tile->type].name);
    }
}

static int draw_examination(const int width, const int max_y)
{
    int row = 0;
    enum item_type type = examined_item;
    if (type == ITEM_TYPE_NONE)
    {
        char description[256];
        describe_target(description, sizeof(description));
        ui_panel_paragraph(&row, overlay_scroll, 2, width, description, max_y);
        const struct tile *tile = &world->maps[world->player->floor].tiles[target_x][target_y];
        if (TCOD_map_is_in_fov(world->player->fov, target_x, target_y) && tile->item)
        {
            type = tile->item->type;
        }
    }
    if (type != ITEM_TYPE_NONE)
    {
        const struct item_data *data = &item_database[type];
        const struct base_item_data *base = &base_item_database[data->type];
        char value[64];
        ui_panel_blank(&row);
        ui_panel_header(&row, overlay_scroll, 2, width, data->name, max_y);
        if (data->description)
        {
            ui_panel_paragraph(&row, overlay_scroll, 2, width, data->description, max_y);
        }
        ui_panel_blank(&row);
        snprintf(value, sizeof(value), "%.1f lb", base->weight);
        ui_panel_value(&row, overlay_scroll, 2, width, "Weight (each)", value, max_y);
        snprintf(value, sizeof(value), "%d gp", base->cost);
        ui_panel_value(&row, overlay_scroll, 2, width, "Value (each)", value, max_y);
    }
    return row;
}

static void draw_examine_overlay(const int max_y, const int max_x)
{
    ui_panel_frame("Examine", "[↑↓/jk] scroll  [PgUp/PgDn] page  [ESC] close");
    const int total_rows = draw_examination(max_x - 4, 0);
    overlay_scroll = ui_panel_scroll(overlay_scroll, total_rows, max_y);
    draw_examination(max_x - 4, max_y);
    ui_panel_scrollbar(total_rows, max_y, max_x, overlay_scroll);
}

static void draw_help_overlay(const int max_y, const int max_x)
{
    static const char *const commands[] = {
        "Movement",
        "Arrows, hjkl, yubn, or 1-9: move (5 or . waits)",
        "g or ,: pick up    <: ascend    >: descend    R: rest",
        "o: open door    O: close door    D: dip    F: attack/bashing direction",
        "Ctrl + arrows/numpad: attack or bash without moving",
        "Inventory",
        "i: inventory    d: drop    e: equip    U: unequip",
        "q: quaff    r: read    L: learn    X: examine carried item",
        "Ctrl-X: examine equipment",
        "Character and spells",
        "c: character sheet    a: spend ability points",
        "v: spellbook    Z: ready spell    z: cast readied spell",
        "Targeting",
        "f: aim/shoot    w: look    x: examine map",
        "Arrows, vi keys, or numpad: move target    Enter: confirm",
        "f/r/z/x also confirm their respective targeting action",
        "Selecting items, spells, abilities, or equipment",
        "a-z: select directly    Up/Down: select    Enter: confirm",
        "PgUp/PgDn: page    Home/End: first/last selection",
        "Other",
        "H: toggle HUD    t/T: toggle torch/glow    Ctrl-K: debug death",
        "ESC: cancel action, close overlay, or save and return to menu",
        "Overlays",
        "Up/Down or j/k: scroll    PgUp/PgDn: page",
        "i/c/v: switch overlays    ?: help",
    };
    ui_panel_frame("Help", "[↑↓/jk] scroll  [PgUp/PgDn] page  [ESC] close");
    int total_rows = 0;
    for (size_t i = 0; i < sizeof(commands) / sizeof(commands[0]); i++)
    {
        ui_panel_paragraph(&total_rows, 0, 2, max_x - 4, commands[i], 0);
    }
    overlay_scroll = ui_panel_scroll(overlay_scroll, total_rows, max_y);
    int row = 0;
    for (size_t i = 0; i < sizeof(commands) / sizeof(commands[0]); i++)
    {
        ui_panel_paragraph(&row, overlay_scroll, 2, max_x - 4, commands[i], max_y);
    }
    ui_panel_scrollbar(total_rows, max_y, max_x, overlay_scroll);
}

static void add_light(float foreground[3], float background[3], const TCOD_Map *fov,
                      int source_x, int source_y, int x, int y, int radius,
                      struct color color, float intensity, bool flicker, float time)
{
    if (!fov || radius <= 0 || !TCOD_map_is_in_fov(fov, x, y))
    {
        return;
    }

    const float dx = (float)(x - source_x);
    const float dy = (float)(y - source_y);
    const float radius_sq = (float)radius * radius;
    float attenuation = 1.0f - (dx * dx + dy * dy) / radius_sq;
    if (flicker)
    {
        attenuation += 0.01f * sinf(time * 10.0f + (float)(source_x * 7 + source_y * 13));
    }
    attenuation = fmaxf(0.0f, fminf(1.0f, attenuation));
    const float components[3] = {color.r, color.g, color.b};
    for (int i = 0; i < 3; i++)
    {
        foreground[i] += components[i] * attenuation;
        background[i] += components[i] * intensity * attenuation;
    }
}

static struct color tone_map(const float light[3], struct color material)
{
    const float maximum = fmaxf(light[0], fmaxf(light[1], light[2]));
    const float scale = maximum > 255.0f ? 255.0f / maximum : 1.0f;
    return (struct color){
        (uint8_t)(light[0] * scale * material.r / 255.0f),
        (uint8_t)(light[1] * scale * material.g / 255.0f),
        (uint8_t)(light[2] * scale * material.b / 255.0f),
    };
}

static void lighting_tile(const struct map *const map, const struct actor *const player,
                          const int x, const int y, const float time,
                          struct color *const foreground, struct color *const background)
{
    const struct color ambient = tile_metadata.ambient_light_color;
    float fg[3] = {ambient.r, ambient.g, ambient.b};
    float bg[3] = {ambient.r * tile_metadata.ambient_light_intensity,
                   ambient.g * tile_metadata.ambient_light_intensity,
                   ambient.b * tile_metadata.ambient_light_intensity};

    if (TCOD_map_is_in_fov(player->fov, x, y))
    {
        for (size_t i = 0; i < map->objects->size; i++)
        {
            const struct object *object = list_get(map->objects, i);
            const struct light_data *light = &light_database[object_database[object->type].light_type];
            add_light(fg, bg, object->light_fov, object->x, object->y, x, y,
                      light->radius, light->color, light->intensity, light->flicker, time);
        }
        for (size_t i = 0; i < map->actors->size; i++)
        {
            const struct actor *actor = list_get(map->actors, i);
            const struct light_data *light = &light_database[actor->light_type];
            const int radius = actor->light_type == LIGHT_TYPE_PLAYER
                                   ? actor_get_sight_radius(actor) : light->radius;
            add_light(fg, bg, actor->light_fov, actor->x, actor->y, x, y,
                      radius, light->color, light->intensity, light->flicker, time);
        }
        for (size_t i = 0; i < map->projectiles->size; i++)
        {
            const struct projectile *projectile = list_get(map->projectiles, i);
            const struct light_data *light = &light_database[projectile_database[projectile->type].light_type];
            add_light(fg, bg, projectile->light_fov, (int)projectile->x, (int)projectile->y, x, y,
                      light->radius, light->color, light->intensity, light->flicker, time);
        }
        for (size_t i = 0; i < map->explosions->size; i++)
        {
            const struct explosion *explosion = list_get(map->explosions, i);
            const struct explosion_data *light = &explosion_database[explosion->type];
            add_light(fg, bg, explosion->fov, explosion->x, explosion->y, x, y,
                      light->radius, light->color, light->intensity, true, time);
        }
        for (size_t i = 0; i < map->surfaces->size; i++)
        {
            const struct surface *surface = list_get(map->surfaces, i);
            const struct light_data *light = &light_database[surface_database[surface->type].light_type];
            add_light(fg, bg, surface->light_fov, surface->x, surface->y, x, y,
                      light->radius, light->color, light->intensity, light->flicker, time);
        }
    }

    const struct color material = tile_database[map->tiles[x][y].type].color;
    *foreground = tone_map(fg, material);
    *background = tone_map(bg, material);
}

static void render(void)
{
    if (!world || !world->player)
    {
        return;
    }

    int max_y, max_x;
    getmaxyx(stdscr, max_y, max_x);

    if (!ui_require_size(12, 5))
    {
        return;
    }

    clear();

    view_width = max_x;
    view_height = hud_visible && max_y > MESSAGE_LOG_HEIGHT + 3 ? max_y - MESSAGE_LOG_HEIGHT : max_y;

    const struct actor *player = world->player;
    view_x = (input_mode == INPUT_TARGET ? target_x : player->x) - view_width / 2;
    view_y = (input_mode == INPUT_TARGET ? target_y : player->y) - view_height / 2;

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

    const struct map *map = &world->maps[player->floor];
    struct color backgrounds[MAP_WIDTH][MAP_HEIGHT] = {0};

    for (int x = view_x; x < view_x + view_width && x < MAP_WIDTH; x++)
    {
        for (int y = view_y; y < view_y + view_height && y < MAP_HEIGHT; y++)
        {
            if (!map_is_inside(x, y))
            {
                continue;
            }

            const struct tile *tile = &map->tiles[x][y];
            const struct tile_data *tile_data = &tile_database[tile->type];

            if (tile->explored)
            {
                int glyph = tile_data->glyph;
                const int screen_x = x - view_x;
                const int screen_y = y - view_y;

                if (tile->type == TILE_TYPE_WALL)
                {
                    glyph = get_wall_glyph(map, x, y);
                }

                const bool in_fov = TCOD_map_is_in_fov(player->fov, x, y);
                struct color foreground, background;
                lighting_tile(map, player, x, y, lighting_time, &foreground, &background);
                backgrounds[x][y] = background;

                if (in_fov)
                {
                    if (tile->actor)
                    {
                        const struct actor *actor = tile->actor;
                        const struct class_data *class_data = &class_database[actor->class];
                        glyph = class_data->glyph;
                        foreground = color_lerp(class_data->color, actor->flash_color, actor->flash_alpha);
                    }
                    else if (tile->object && (tile->object->type != OBJECT_TYPE_TRAP ||
                                              tile->object->trap_detection_state == OBJECT_TRAP_DETECTION_STATE_DETECTED))
                    {
                        const struct object_data *object_data = &object_database[tile->object->type];

                        glyph = object_data->glyph;
                        foreground = object_data->color;
                    }

                    else if (tile->item)
                    {
                        const struct item_data *const item_data = &item_database[tile->item->type];
                        const struct base_item_data *const base_item_data = &base_item_database[item_data->type];

                        glyph = base_item_data->glyph;
                        foreground = item_data->color;
                    }
                    else if (tile->corpse)
                    {
                        glyph = corpse_metadata.glyph;
                        foreground = corpse_metadata.color;
                    }
                    else if (tile->surface)
                    {
                        const struct surface_data *const surface_data = &surface_database[tile->surface->type];
                        foreground = surface_data->color;

                        if (surface_data->glyph)
                        {
                            glyph = surface_data->glyph;
                        }
                    }
                }
                terminal_set_color(foreground, background, in_fov ? A_BOLD : 0);
                mvaddch(screen_y, screen_x, glyph);
                terminal_reset_color();
            }
        }
    }

    for (size_t i = 0; i < map->projectiles->size; i++)
    {
        const struct projectile *projectile = list_get(map->projectiles, i);
        const int x = (int)projectile->x;
        const int y = (int)projectile->y;
        if (map_is_inside(x, y) && TCOD_map_is_in_fov(player->fov, x, y) &&
            x >= view_x && x < view_x + view_width && y >= view_y && y < view_y + view_height)
        {
            int glyph = projectile_database[projectile->type].glyph;
            if (projectile->type == PROJECTILE_TYPE_ARROW)
            {
                const int dx = projectile->target_x - projectile->origin_x;
                const int dy = projectile->target_y - projectile->origin_y;
                if (dx == 0)
                {
                    glyph = '|';
                }
                else if (dy == 0)
                {
                    glyph = '-';
                }
                else
                {
                    glyph = (dx > 0) == (dy > 0) ? '\\' : '/';
                }
            }
            terminal_set_color(projectile_database[projectile->type].color, backgrounds[x][y], A_BOLD);
            mvaddch(y - view_y, x - view_x, glyph);
            terminal_reset_color();
        }
    }

    if (active_overlay == OVERLAY_INVENTORY)
    {
        draw_inventory_overlay(max_y, max_x);
    }
    else if (active_overlay == OVERLAY_CHARACTER)
    {
        draw_character_sheet_overlay(max_y, max_x);
    }
    else if (active_overlay == OVERLAY_SPELLBOOK)
    {
        draw_spellbook_overlay(max_y, max_x);
    }
    else if (active_overlay == OVERLAY_HELP)
    {
        draw_help_overlay(max_y, max_x);
    }
    else if (active_overlay == OVERLAY_EXAMINE)
    {
        draw_examine_overlay(max_y, max_x);
    }

    if ((active_overlay == OVERLAY_INVENTORY || active_overlay == OVERLAY_CHARACTER ||
         active_overlay == OVERLAY_SPELLBOOK) &&
        overlay_notice[0])
    {
        ui_panel_notice(overlay_notice);
    }

    if (active_overlay == OVERLAY_NONE && view_height < max_y)
    {
        const int status_width = max_x > STATUS_WIDTH + 10 ? STATUS_WIDTH : max_x / 2;
        attron(A_DIM);
        for (int x = 0; x < max_x; x++)
        {
            mvaddch(view_height, x, ACS_HLINE);
        }
        attroff(A_DIM);

        {
            const int max_hit_points = actor_get_max_hit_points(world->player);
            int row = view_height + 1;
            char text[64];

            snprintf(text, sizeof(text), "HP: %d/%d", world->player->hit_points, max_hit_points);
            const float hp_ratio = max_hit_points > 0
                                       ? fmaxf(0.0f, fminf(1.0f, (float)world->player->hit_points / max_hit_points))
                                       : 0.0f;
            const struct color red = {255, 0, 0};
            const struct color yellow = {255, 255, 0};
            const struct color green = {0, 255, 0};
            const struct color black = {0, 0, 0};
            const struct color hp_color = hp_ratio < 0.5f
                                              ? color_lerp(red, yellow, hp_ratio * 2)
                                              : color_lerp(yellow, green, (hp_ratio - 0.5f) * 2);
            terminal_set_color(hp_color, black, 0);
            mvaddnstr(row++, 0, text, status_width);
            terminal_reset_color();

            if (class_database[world->player->class].spellcasting_ability != ABILITY_NONE)
            {
                const int max_mana = actor_get_max_mana(world->player);
                snprintf(text, sizeof(text), "MP: %d/%d", world->player->mana, max_mana);
                const float mana_ratio = max_mana > 0
                                             ? fmaxf(0.0f, fminf(1.0f, (float)world->player->mana / max_mana))
                                             : 0.0f;
                terminal_set_color(color_lerp((struct color){127, 127, 127},
                                              (struct color){0, 127, 255}, mana_ratio), black, 0);
                mvaddnstr(row++, 0, text, status_width);
                terminal_reset_color();
            }

            snprintf(text, sizeof(text), "CL: %d%s", world->player->level, world->player->ability_points > 0 ? "*" : "");
            mvaddnstr(row++, 0, text, status_width);

            if (row < max_y)
            {
                snprintf(text, sizeof(text), "DL: %d", world->player->floor + 1);
                mvaddnstr(row++, 0, text, status_width);
            }
        }

        for (int y = view_height; y < max_y; y++)
        {
            mvaddch(y, status_width, y == view_height ? ACS_TTEE : ACS_VLINE);
        }

        if (message_count() > 0)
        {
            const int message_width = max_x - status_width - 1;
            int first_message = (int)message_count() - MESSAGE_LOG_HEIGHT + 1;
            if (first_message < 0)
            {
                first_message = 0;
            }

            int log_y = view_height + 1;
            for (int i = first_message; i < (int)message_count() && log_y < max_y; i++, log_y++)
            {
                const struct message *const message = message_get((size_t)i);
                const float age = (float)(i - first_message + 1) / (message_count() - first_message);
                const struct color color = color_lerp((struct color){127, 127, 127},
                                                      message_color(message->type), age);
                terminal_set_color(color, (struct color){0, 0, 0}, 0);
                mvprintw(log_y, status_width + 1, "%.*s", message_width, message->text);
                terminal_reset_color();
                clrtoeol();
            }
        }
    }

    if (active_overlay == OVERLAY_NONE)
    {
        if (input_mode == INPUT_DIRECTION || input_mode == INPUT_TARGET)
        {
            mvaddnstr(0, 0, action_names[current_action], max_x);
            mvaddnstr(1, 0, input_mode == INPUT_DIRECTION ? "[Direction] confirm  [ESC] cancel" : "[Direction] move target  [Enter] confirm  [ESC] cancel", max_x);
            if (input_mode == INPUT_TARGET)
            {
                char description[256];
                describe_target(description, sizeof(description));
                mvaddnstr(2, 0, description, max_x);
                if (target_x >= view_x && target_x < view_x + view_width &&
                    target_y >= view_y && target_y < view_y + view_height)
                {
                    mvchgat(target_y - view_y, target_x - view_x, 1, A_REVERSE | A_BOLD, 0, NULL);
                }
            }
        }
        else if (world->doomed)
        {
            mvaddnstr(0, 0, "Game over. [ESC] menu", max_x);
        }
        else if (!world_can_player_take_turn())
        {
            mvaddnstr(0, 0, "Waiting...", max_x);
        }
    }
    refresh();
}

static struct scene *update(const float delta_time)
{
    if (!world || !world->hero)
    {
        fprintf(stderr, "Cannot enter the game without a loaded character.\n");
        return &scene_menu;
    }
    if (!world->player)
    {
        world->player = world->hero;
    }

    // Handle input first so took_turn is set before world_update processes turns
    collecting_feedback = true;
    struct scene *next = input();
    collecting_feedback = false;
    if (next != &scene_game)
    {
        return next;
    }

    const bool was_doomed = world->doomed;
    world_update(delta_time);
    lighting_time = fmodf(lighting_time + delta_time, 1000.0f);
    if (action_actor && (action_actor != world->player || world->doomed || world->player->dead))
    {
        cancel_action();
    }
    if (!was_doomed && world->doomed && remove(SAVE_PATH) != 0 && errno != ENOENT)
    {
        fprintf(stderr, "Could not delete save %s: %s\n", SAVE_PATH, strerror(errno));
    }

    render();

    return &scene_game;
}

struct scene scene_game = {
    .init = init,
    .uninit = uninit,
    .update = update,
};
