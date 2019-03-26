#include <platform/platform.h>

// TODO: maybe split this up into multiple files? might not be necessary

// TODO: better mouse controls
// design a system where when selecting an action from the right-click menu, other actions are ignored as the actor paths to the location
// when left-clicking a hostile actor, or selecting Attack from the menu, should track that actor's position and follow it. when reached, attack once and stop automoving

// TODO: automoving needs to be slower and easy to cancel

// TODO: better sanity checking for tooltip data
// right now, tooltip data is contained in a module scope struct
// need to define ways to make sure the struct is valid for the given tooltip options

// TODO: prompts

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

enum inventory_action
{
    INVENTORY_ACTION_NONE,
    INVENTORY_ACTION_DROP,
    INVENTORY_ACTION_EQUIP,
    INVENTORY_ACTION_EXAMINE,
    INVENTORY_ACTION_QUAFF
};

enum character_action
{
    CHARACTER_ACTION_NONE,
    CHARACTER_ACTION_EXAMINE,
    CHARACTER_ACTION_UNEQUIP
};

enum targeting
{
    TARGETING_NONE,
    TARGETING_LOOK,
    TARGETING_EXAMINE,
    TARGETING_SHOOT,
    TARGETING_SPELL
};

enum panel
{
    PANEL_CHARACTER,
    PANEL_EXAMINE,
    PANEL_INVENTORY,
    PANEL_SPELLBOOK,

    NUM_PANELS
};

struct panel_status
{
    int scroll;
    bool selection_mode;
};

struct tooltip_option
{
    char *text;
    bool(*on_click)(void);
};

struct tooltip_data
{
    int x;
    int y;
    struct object *object;
    struct item *item;
    struct actor *actor;
    enum equip_slot equip_slot;
};

static enum directional_action directional_action;
static enum inventory_action inventory_action;
static enum character_action character_action;

static bool automoving;
static int automove_x;
static int automove_y;
static struct actor *automove_actor;

static bool took_turn;

static enum targeting targeting;
static int target_x;
static int target_y;

static enum panel_type current_panel;
static struct panel_status panel_status[NUM_PANELS];

static TCOD_console_t message_log;
static bool message_log_visible;
static int message_log_x;
static int message_log_height;
static int message_log_y;
static int message_log_width;

static TCOD_console_t panel;
static bool panel_visible;
static int panel_width;
static int panel_x;
static int panel_y;
static int panel_height;

static TCOD_console_t tooltip;
static bool tooltip_visible;
static int tooltip_x;
static int tooltip_y;
static int tooltip_width;
static int tooltip_height;
static TCOD_list_t tooltip_options;
static struct tooltip_data tooltip_data;

static int view_width;
static int view_height;
static int view_x;
static int view_y;

static int mouse_x;
static int mouse_y;
static int mouse_tile_x;
static int mouse_tile_y;

static TCOD_noise_t noise;
static float noise_x;

static void init(struct state *previous_state);
static struct state *handleEvent(TCOD_event_t ev, TCOD_key_t key, TCOD_mouse_t mouse);
static struct state *update(float delta);
static void render(TCOD_console_t console);
static void quit(void);
static bool do_directional_action(struct actor *player, int x, int y);
static void on_hit_set_took_turn(void *on_hit_params);
static bool toolip_on_click_move(void);
static bool message_log_is_inside(int x, int y);
static bool panel_is_inside(int x, int y);
static void panel_toggle(enum panel panel);
static void panel_show(enum panel panel);
static enum equip_slot panel_character_get_selected(void);
static struct item *panel_inventory_get_selected(void);
static bool tooltip_is_inside(int x, int y);
static void tooltip_show(void);
static void tooltip_hide(void);
struct tooltip_option *tooltip_option_create(char *text, bool(*on_click)(void));
void tooltip_option_destroy(struct tooltip_option *tooltip_option);
static void tooltip_options_add(char *text, bool(*on_click)(void));
static void tooltip_options_clear(void);
static struct tooltip_option *tooltip_get_selected(void);
static bool view_is_inside(int x, int y);

struct state game_state = {
    &init,
    &handleEvent,
    &update,
    &render,
    &quit
};

static void init(struct state *previous_state)
{
    directional_action = DIRECTIONAL_ACTION_NONE;
    inventory_action = INVENTORY_ACTION_NONE;
    character_action = CHARACTER_ACTION_NONE;

    automoving = false;
    automove_x = -1;
    automove_y = -1;
    automove_actor = NULL;

    took_turn = true;

    targeting = TARGETING_NONE;
    target_x = -1;
    target_y = -1;

    current_panel = PANEL_CHARACTER;

    panel_status[PANEL_CHARACTER].scroll = 0;
    panel_status[PANEL_CHARACTER].selection_mode = false;
    panel_status[PANEL_EXAMINE].scroll = 0;
    panel_status[PANEL_EXAMINE].selection_mode = false;
    panel_status[PANEL_INVENTORY].scroll = 0;
    panel_status[PANEL_INVENTORY].selection_mode = false;
    panel_status[PANEL_SPELLBOOK].scroll = 0;
    panel_status[PANEL_SPELLBOOK].selection_mode = false;

    panel = TCOD_console_new(console_width, console_height);
    message_log_visible = true;
    message_log_x = 0;
    message_log_height = 0;
    message_log_y = 0;
    message_log_width = 0;

    message_log = TCOD_console_new(console_width, console_height);
    panel_visible = false;
    panel_width = 0;
    panel_x = 0;
    panel_y = 0;
    panel_height = 0;

    tooltip = TCOD_console_new(console_width, console_height);
    tooltip_visible = false;
    tooltip_x = 0;
    tooltip_y = 0;
    tooltip_width = 0;
    tooltip_height = 0;
    tooltip_options = TCOD_list_new();

    view_width = 0;
    view_height = 0;
    view_x = 0;
    view_y = 0;

    mouse_x = 0;
    mouse_y = 0;
    mouse_tile_x = 0;
    mouse_tile_y = 0;

    noise = TCOD_noise_new(1, TCOD_NOISE_DEFAULT_HURST, TCOD_NOISE_DEFAULT_LACUNARITY, NULL);
    noise_x = 0.0f;
}

static struct state *handleEvent(TCOD_event_t ev, TCOD_key_t key, TCOD_mouse_t mouse)
{
    switch (ev)
    {
    case TCOD_EVENT_KEY_PRESS:
    {
        automoving = false;

        switch (key.vk)
        {
        case TCODK_ESCAPE:
        {
            if (tooltip_visible)
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
                    panel_status[panel].selection_mode = false;
                }
            }
            else if (panel_visible)
            {
                panel_visible = false;
            }
            else if (targeting)
            {
                targeting = false;
            }
            else
            {
                game_state.quit();
                menu_state.init(&game_state);
                return &menu_state;
            }
        }
        break;
        case TCODK_PAGEDOWN:
        {
            if (panel_visible)
            {
                struct panel_status *current_panel_status = &panel_status[current_panel];

                current_panel_status->scroll++;
            }
        }
        break;
        case TCODK_PAGEUP:
        {
            if (panel_visible)
            {
                struct panel_status *current_panel_status = &panel_status[current_panel];

                current_panel_status->scroll--;
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
            else if (game->state == GAME_STATE_PLAY)
            {
                int x = game->player->x - 1;
                int y = game->player->y + 1;

                if (directional_action == DIRECTIONAL_ACTION_NONE)
                {
                    if (key.lctrl)
                    {
                        took_turn = actor_swing(game->player, x, y);
                    }
                    else
                    {
                        took_turn = actor_move(game->player, x, y);
                    }
                }
                else
                {
                    took_turn = do_directional_action(game->player, x, y);
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
            else if (game->state == GAME_STATE_PLAY)
            {
                int x = game->player->x;
                int y = game->player->y + 1;

                if (directional_action == DIRECTIONAL_ACTION_NONE)
                {
                    if (key.lctrl)
                    {
                        took_turn = actor_swing(game->player, x, y);
                    }
                    else
                    {
                        took_turn = actor_move(game->player, x, y);
                    }
                }
                else
                {
                    took_turn = do_directional_action(game->player, x, y);
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
            else if (game->state == GAME_STATE_PLAY)
            {
                int x = game->player->x + 1;
                int y = game->player->y + 1;

                if (directional_action == DIRECTIONAL_ACTION_NONE)
                {
                    if (key.lctrl)
                    {
                        took_turn = actor_swing(game->player, x, y);
                    }
                    else
                    {
                        took_turn = actor_move(game->player, x, y);
                    }
                }
                else
                {
                    took_turn = do_directional_action(game->player, x, y);
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
            else if (game->state == GAME_STATE_PLAY)
            {
                int x = game->player->x - 1;
                int y = game->player->y;

                if (directional_action == DIRECTIONAL_ACTION_NONE)
                {
                    if (key.lctrl)
                    {
                        took_turn = actor_swing(game->player, x, y);
                    }
                    else
                    {
                        took_turn = actor_move(game->player, x, y);
                    }
                }
                else
                {
                    took_turn = do_directional_action(game->player, x, y);
                }
            }
        }
        break;
        case TCODK_KP5:
        {
            if (game->state != GAME_STATE_WAIT)
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
            else if (game->state == GAME_STATE_PLAY)
            {
                int x = game->player->x + 1;
                int y = game->player->y;

                if (directional_action == DIRECTIONAL_ACTION_NONE)
                {
                    if (key.lctrl)
                    {
                        took_turn = actor_swing(game->player, x, y);
                    }
                    else
                    {
                        took_turn = actor_move(game->player, x, y);
                    }
                }
                else
                {
                    took_turn = do_directional_action(game->player, x, y);
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
            else if (game->state == GAME_STATE_PLAY)
            {
                int x = game->player->x - 1;
                int y = game->player->y - 1;

                if (directional_action == DIRECTIONAL_ACTION_NONE)
                {
                    if (key.lctrl)
                    {
                        took_turn = actor_swing(game->player, x, y);
                    }
                    else
                    {
                        took_turn = actor_move(game->player, x, y);
                    }
                }
                else
                {
                    took_turn = do_directional_action(game->player, x, y);
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
            else if (game->state == GAME_STATE_PLAY)
            {
                int x = game->player->x;
                int y = game->player->y - 1;

                if (directional_action == DIRECTIONAL_ACTION_NONE)
                {
                    if (key.lctrl)
                    {
                        took_turn = actor_swing(game->player, x, y);
                    }
                    else
                    {
                        took_turn = actor_move(game->player, x, y);
                    }
                }
                else
                {
                    took_turn = do_directional_action(game->player, x, y);
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
            else if (game->state == GAME_STATE_PLAY)
            {
                int x = game->player->x + 1;
                int y = game->player->y - 1;

                if (directional_action == DIRECTIONAL_ACTION_NONE)
                {
                    if (key.lctrl)
                    {
                        took_turn = actor_swing(game->player, x, y);
                    }
                    else
                    {
                        took_turn = actor_move(game->player, x, y);
                    }
                }
                else
                {
                    took_turn = do_directional_action(game->player, x, y);
                }
            }
        }
        break;
        case TCODK_TEXT:
        {
            bool handled = false;
            int alpha = key.text[0] - 'a';

            if (inventory_action != INVENTORY_ACTION_NONE && alpha >= 0 && alpha < TCOD_list_size(game->player->items))
            {
                struct item *item = TCOD_list_get(game->player->items, alpha);

                switch (inventory_action)
                {
                case INVENTORY_ACTION_EQUIP:
                {
                    if (game->state == GAME_STATE_PLAY)
                    {
                        took_turn = actor_equip(game->player, item);
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
                    if (game->state == GAME_STATE_PLAY)
                    {
                        took_turn = actor_drop(game->player, item);
                    }
                }
                break;
                case INVENTORY_ACTION_QUAFF:
                {
                    if (game->state == GAME_STATE_PLAY)
                    {
                        took_turn = actor_quaff(game->player, item);
                    }
                }
                break;
                }

                inventory_action = INVENTORY_ACTION_NONE;

                panel_status[PANEL_INVENTORY].selection_mode = false;

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
                    if (game->state == GAME_STATE_PLAY)
                    {
                        took_turn = actor_unequip(game->player, equip_slot);
                    }
                }
                break;
                }

                character_action = CHARACTER_ACTION_NONE;

                panel_status[PANEL_CHARACTER].selection_mode = false;

                handled = true;
            }

            for (enum panel panel = 0; panel < NUM_PANELS; panel++)
            {
                if (panel_status[panel].selection_mode)
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
                if (game->state == GAME_STATE_PLAY)
                {
                    took_turn = actor_ascend(game->player);
                }
            }
            break;
            case '>':
            {
                if (game->state == GAME_STATE_PLAY)
                {
                    took_turn = actor_descend(game->player);
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
                if (game->state == GAME_STATE_PLAY)
                {
                    directional_action = DIRECTIONAL_ACTION_CLOSE_DOOR;

                    game_log(
                        game->player->floor,
                        game->player->x,
                        game->player->y,
                        TCOD_white,
                        "Choose a direction, ESC to cancel");
                }
            }
            break;
            case 'D':
            {
                if (game->state == GAME_STATE_PLAY)
                {
                    directional_action = DIRECTIONAL_ACTION_DRINK;

                    game_log(
                        game->player->floor,
                        game->player->x,
                        game->player->y,
                        TCOD_white,
                        "Choose a direction, ESC to cancel");
                }
            }
            break;
            case 'd':
            {
                if (game->state == GAME_STATE_PLAY)
                {
                    panel_show(PANEL_INVENTORY);

                    inventory_action = INVENTORY_ACTION_DROP;
                    panel_status[PANEL_INVENTORY].selection_mode = true;

                    game_log(
                        game->player->floor,
                        game->player->x,
                        game->player->y,
                        TCOD_white,
                        "Choose an item to drop, ESC to cancel");
                }
            }
            break;
            case 'e':
            {
                if (game->state == GAME_STATE_PLAY)
                {
                    panel_show(PANEL_INVENTORY);

                    inventory_action = INVENTORY_ACTION_EQUIP;
                    panel_status[PANEL_INVENTORY].selection_mode = true;

                    game_log(
                        game->player->floor,
                        game->player->x,
                        game->player->y,
                        TCOD_white,
                        "Choose an item to equip, ESC to cancel");
                }
            }
            break;
            case 'f':
            {
                if (game->state == GAME_STATE_PLAY)
                {
                    if (targeting == TARGETING_SHOOT)
                    {
                        actor_shoot(game->player, target_x, target_y, &on_hit_set_took_turn, NULL);

                        targeting = TARGETING_NONE;
                    }
                    else
                    {
                        targeting = TARGETING_SHOOT;

                        bool target_found = false;

                        struct map *map = &game->maps[game->player->floor];

                        {
                            struct actor *target = NULL;
                            float min_distance = FLT_MAX;

                            TCOD_LIST_FOREACH(map->actors)
                            {
                                struct actor *actor = *iterator;

                                if (TCOD_map_is_in_fov(game->player->fov, actor->x, actor->y) &&
                                    actor->faction != game->player->faction &&
                                    !actor->dead)
                                {
                                    float distance = distance_between_sq(game->player->x, game->player->y, actor->x, actor->y);

                                    if (distance < min_distance)
                                    {
                                        target = actor;
                                        min_distance = distance;
                                    }
                                }
                            }

                            if (target)
                            {
                                target_found = true;

                                target_x = target->x;
                                target_y = target->y;
                            }
                        }

                        if (!target_found)
                        {
                            target_x = game->player->x;
                            target_y = game->player->y;
                        }
                    }
                }
            }
            break;
            case 'g':
            {
                if (game->state == GAME_STATE_PLAY)
                {
                    took_turn = actor_grab(game->player, game->player->x, game->player->y);
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

                    target_x = game->player->x;
                    target_y = game->player->y;
                }
            }
            break;
            case 'm':
            {
                message_log_visible = !message_log_visible;
            }
            break;
            case 'O':
            {
                if (game->state == GAME_STATE_PLAY)
                {
                    directional_action = DIRECTIONAL_ACTION_OPEN_CHEST;

                    game_log(
                        game->player->floor,
                        game->player->x,
                        game->player->y,
                        TCOD_white,
                        "Choose a direction, ESC to cancel");
                }
            }
            break;
            case 'o':
            {
                if (game->state == GAME_STATE_PLAY)
                {
                    directional_action = DIRECTIONAL_ACTION_OPEN_DOOR;

                    game_log(
                        game->player->floor,
                        game->player->x,
                        game->player->y,
                        TCOD_white,
                        "Choose a direction, ESC to cancel");
                }
            }
            break;
            case 'p':
            {
                if (game->state == GAME_STATE_PLAY)
                {
                    directional_action = DIRECTIONAL_ACTION_PRAY;

                    game_log(
                        game->player->floor,
                        game->player->x,
                        game->player->y,
                        TCOD_white,
                        "Choose a direction, ESC to cancel");
                }
            }
            break;
            case 'q':
            {
                if (game->state == GAME_STATE_PLAY)
                {
                    panel_show(PANEL_INVENTORY);

                    inventory_action = INVENTORY_ACTION_QUAFF;
                    panel_status[PANEL_INVENTORY].selection_mode = true;

                    game_log(
                        game->player->floor,
                        game->player->x,
                        game->player->y,
                        TCOD_white,
                        "Choose an item to quaff, ESC to cancel");
                }
            }
            break;
            case 's':
            {
                if (game->state == GAME_STATE_PLAY)
                {
                    directional_action = DIRECTIONAL_ACTION_SIT;

                    game_log(
                        game->player->floor,
                        game->player->x,
                        game->player->y,
                        TCOD_white,
                        "Choose a direction, ESC to cancel");
                }
            }
            break;
            case 't':
            {
                if (game->state == GAME_STATE_PLAY)
                {
                    game->player->torch = !game->player->torch;

                    took_turn = true;
                }
            }
            break;
            case 'u':
            {
                if (game->state == GAME_STATE_PLAY)
                {
                    panel_show(PANEL_CHARACTER);

                    character_action = CHARACTER_ACTION_UNEQUIP;
                    panel_status[PANEL_CHARACTER].selection_mode = true;

                    game_log(
                        game->player->floor,
                        game->player->x,
                        game->player->y,
                        TCOD_white,
                        "Choose an item to unequip, ESC to cancel");
                }
            }
            break;
            case 'X':
            {
                panel_show(PANEL_INVENTORY);

                inventory_action = INVENTORY_ACTION_EXAMINE;
                panel_status[PANEL_INVENTORY].selection_mode = true;

                game_log(
                    game->player->floor,
                    game->player->x,
                    game->player->y,
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
                    panel_status[PANEL_CHARACTER].selection_mode = true;

                    game_log(
                        game->player->floor,
                        game->player->x,
                        game->player->y,
                        TCOD_white,
                        "Choose an equipment to examine, ESC to cancel");
                }
                else
                {
                    if (targeting == TARGETING_EXAMINE)
                    {
                        targeting = TARGETING_NONE;

                        // TODO: send examine target to ui

                        panel_show(PANEL_EXAMINE);
                    }
                    else
                    {
                        targeting = TARGETING_EXAMINE;

                        target_x = game->player->x;
                        target_y = game->player->y;
                    }
                }
            }
            break;
            case 'z':
            {
                if (game->state == GAME_STATE_PLAY)
                {
                    if (targeting == TARGETING_SPELL)
                    {
                        targeting = TARGETING_NONE;
                    }
                    else
                    {
                        targeting = TARGETING_SPELL;

                        target_x = game->player->x;
                        target_y = game->player->y;
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

            if (tooltip_visible)
            {
                if (tooltip_is_inside(mouse_x, mouse_y))
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

                struct tile *tile = &game->maps[game->player->floor].tiles[mouse_tile_x][mouse_tile_y];

                if (tile->actor && tile->actor->faction != game->player->faction && !tile->actor->dead)
                {
                    automove_actor = tile->actor;
                }
                else
                {
                    automove_x = mouse_tile_x;
                    automove_y = mouse_tile_y;
                }
            }
            else if (panel_is_inside(mouse_x, mouse_y))
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
                            if (game->state == GAME_STATE_PLAY)
                            {
                                took_turn = actor_equip(game->player, item);
                            }
                        }
                        break;
                        case INVENTORY_ACTION_DROP:
                        {
                            if (game->state == GAME_STATE_PLAY)
                            {
                                took_turn = actor_drop(game->player, item);
                            }
                        }
                        break;
                        }

                        inventory_action = INVENTORY_ACTION_NONE;

                        panel_status[PANEL_INVENTORY].selection_mode = false;
                    }
                }
                else if (character_action != CHARACTER_ACTION_NONE)
                {
                    enum equip_slot equip_slot = panel_character_get_selected();

                    if (equip_slot >= 1 && equip_slot < NUM_EQUIP_SLOTS)
                    {
                        if (game->state == GAME_STATE_PLAY)
                        {
                            took_turn = actor_unequip(game->player, equip_slot);
                        }
                    }

                    character_action = CHARACTER_ACTION_NONE;

                    panel_status[PANEL_CHARACTER].selection_mode = false;
                }
            }
        }
        else if (mouse.rbutton)
        {
            if (view_is_inside(mouse_x, mouse_y) && map_is_inside(mouse_tile_x, mouse_tile_y))
            {
                struct map *map = &game->maps[game->player->floor];
                struct tile *tile = &map->tiles[mouse_tile_x][mouse_tile_y];

                tooltip_show();

                tooltip_options_add("Move", &toolip_on_click_move);

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
            else if (panel_is_inside(mouse_x, mouse_y))
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

                        if (base_item_info[item_info[item->type].base_item].equip_slot != EQUIP_SLOT_NONE)
                        {
                            tooltip_options_add("Equip", NULL);
                        }

                        if (item_info[item->type].base_item == BASE_ITEM_POTION)
                        {
                            tooltip_options_add("Quaff", NULL);
                        }

                        tooltip_options_add("Cancel", NULL);

                        tooltip_data.item = item;
                    }

                    break;
                }
                break;
                case PANEL_CHARACTER:
                {
                    enum equip_slot equip_slot = panel_character_get_selected();

                    if (equip_slot >= 1 && equip_slot < NUM_EQUIP_SLOTS)
                    {
                        struct item *equipment = game->player->equipment[equip_slot];

                        if (equipment)
                        {
                            tooltip_show();

                            tooltip_options_add("Unequip", NULL);

                            tooltip_options_add("Cancel", NULL);

                            tooltip_data.equip_slot = equip_slot;
                        }
                    }
                }
                break;
                }
            }
        }
        else if (mouse.wheel_down)
        {
            if (panel_visible)
            {
                struct panel_status *current_panel_status = &panel_status[current_panel];

                current_panel_status->scroll++;
            }
        }
        else if (mouse.wheel_up)
        {
            if (panel_visible)
            {
                struct panel_status *current_panel_status = &panel_status[current_panel];

                current_panel_status->scroll--;
            }
        }
    }
    break;
    }

    if (automoving && game->state == GAME_STATE_PLAY)
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
        took_turn = actor_path_towards(game->player, automove_x, automove_y);

        if (!took_turn)
        {
            automoving = false;
            automove_x = -1;
            automove_y = -1;
            automove_actor = NULL;
        }
    }

    mouse_x = mouse.cx;
    mouse_y = mouse.cy;
    mouse_tile_x = mouse.cx + view_x;
    mouse_tile_y = mouse.cy + view_y;

    return &game_state;
}

static struct state *update(float delta)
{
    message_log_x = 0;
    message_log_height = console_height / 4;
    message_log_y = console_height - message_log_height;
    message_log_width = console_width;

    panel_width = console_width / 2;
    panel_x = console_width - panel_width;
    panel_y = 0;
    panel_height = console_height - (message_log_visible ? message_log_height : 0);

    tooltip_width = 0;
    TCOD_LIST_FOREACH(tooltip_options)
    {
        struct tooltip_option *option = *iterator;

        int len = (int)strlen(option->text) + 2;

        if (len > tooltip_width)
        {
            tooltip_width = len;
        }
    }
    tooltip_height = TCOD_list_size(tooltip_options) + 2;

    view_width = console_width - (panel_visible ? panel_width : 0);
    view_height = console_height - (message_log_visible ? message_log_height : 0);
    view_x = game->player->x - view_width / 2;
    view_y = game->player->y - view_height / 2;

    if (view_x + view_width > MAP_WIDTH)
        view_x = MAP_WIDTH - view_width;
    if (view_x < 0)
        view_x = 0;
    if (view_y + view_height > MAP_HEIGHT)
        view_y = MAP_HEIGHT - view_height;
    if (view_y < 0)
        view_y = 0;

    game_update();

    if (took_turn)
    {
        took_turn = false;

        game_turn();
    }

    if (game->state == GAME_STATE_LOSE && file_exists(SAVE_PATH))
    {
        remove(SAVE_PATH);
    }

    return &game_state;
}

static void render(TCOD_console_t console)
{
    struct map *map = &game->maps[game->player->floor];

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

                    TCOD_color_t color = tile_common.shadow_color;

                    if (TCOD_map_is_in_fov(game->player->fov, x, y))
                    {
                        tile->seen = true;

                        TCOD_LIST_FOREACH(map->actors)
                        {
                            struct actor *actor = *iterator;

                            if (actor->glow_fov && TCOD_map_is_in_fov(actor->glow_fov, x, y))
                            {
                                float r2 = powf((float)actor_common.glow_radius, 2);
                                float d = powf((float)(x - actor->x), 2) + powf((float)(y - actor->y), 2);
                                float l = CLAMP(0.0f, 1.0f, (r2 - d) / r2);

                                color = TCOD_color_lerp(color, TCOD_color_lerp(tile_info[tile->type].color, actor_common.glow_color, l), l);
                            }
                        }

                        TCOD_LIST_FOREACH(map->objects)
                        {
                            struct object *object = *iterator;

                            if (object->light_fov && TCOD_map_is_in_fov(object->light_fov, x, y))
                            {
                                float r2 = powf((float)object->light_radius, 2);
                                float d = powf((float)(x - object->x + (object->light_flicker ? dx : 0)), 2) + powf((float)(y - object->y + (object->light_flicker ? dy : 0)), 2);
                                float l = CLAMP(0.0f, 1.0f, (r2 - d) / r2 + (object->light_flicker ? di : 0));

                                color = TCOD_color_lerp(color, TCOD_color_lerp(tile_info[tile->type].color, object->light_color, l), l);
                            }
                        }

                        TCOD_LIST_FOREACH(map->actors)
                        {
                            struct actor *actor = *iterator;

                            if (actor->torch_fov && TCOD_map_is_in_fov(actor->torch_fov, x, y))
                            {
                                float r2 = powf((float)actor_common.torch_radius, 2);
                                float d = powf(x - actor->x + dx, 2) + powf(y - actor->y + dy, 2);
                                float l = CLAMP(0.0f, 1.0f, (r2 - d) / r2 + di);

                                color = TCOD_color_lerp(color, TCOD_color_lerp(tile_info[tile->type].color, actor_common.torch_color, l), l);
                            }
                        }
                    }

                    if (tile->seen)
                    {
                        TCOD_console_set_char_foreground(console, x - view_x, y - view_y, color);
                        TCOD_console_set_char(console, x - view_x, y - view_y, tile_info[tile->type].glyph);
                    }
                }
            }
        }
    }

    TCOD_LIST_FOREACH(map->actors)
    {
        struct actor *actor = *iterator;

        if (actor->dead && TCOD_map_is_in_fov(game->player->fov, actor->x, actor->y))
        {
            TCOD_console_set_char_foreground(console, actor->x - view_x, actor->y - view_y, TCOD_dark_red);
            TCOD_console_set_char(console, actor->x - view_x, actor->y - view_y, '%');
        }
    }

    TCOD_LIST_FOREACH(map->objects)
    {
        struct object *object = *iterator;

        if (TCOD_map_is_in_fov(game->player->fov, object->x, object->y))
        {
            TCOD_console_set_char_foreground(console, object->x - view_x, object->y - view_y, object->color);
            TCOD_console_set_char(console, object->x - view_x, object->y - view_y, object_info[object->type].glyph);
        }
    }

    TCOD_LIST_FOREACH(map->items)
    {
        struct item *item = *iterator;

        if (TCOD_map_is_in_fov(game->player->fov, item->x, item->y))
        {
            TCOD_console_set_char_foreground(console, item->x - view_x, item->y - view_y, base_item_info[item_info[item->type].base_item].color);
            TCOD_console_set_char(console, item->x - view_x, item->y - view_y, base_item_info[item_info[item->type].base_item].glyph);
        }
    }

    TCOD_LIST_FOREACH(map->projectiles)
    {
        struct projectile *projectile = *iterator;

        int x = (int)projectile->x;
        int y = (int)projectile->y;

        if (TCOD_map_is_in_fov(game->player->fov, x, y))
        {
            TCOD_console_set_char_foreground(console, x - view_x, y - view_y, TCOD_white);
            TCOD_console_set_char(console, x - view_x, y - view_y, projectile->glyph);
        }
    }

    TCOD_LIST_FOREACH(map->objects)
    {
        struct object *object = *iterator;

        if ((object->type == OBJECT_TYPE_STAIR_DOWN || object->type == OBJECT_TYPE_STAIR_UP) && TCOD_map_is_in_fov(game->player->fov, object->x, object->y))
        {
            TCOD_console_set_char_foreground(console, object->x - view_x, object->y - view_y, object->color);
            TCOD_console_set_char(console, object->x - view_x, object->y - view_y, object_info[object->type].glyph);
        }
    }

    TCOD_LIST_FOREACH(map->actors)
    {
        struct actor *actor = *iterator;

        if (!actor->dead && TCOD_map_is_in_fov(game->player->fov, actor->x, actor->y))
        {
            TCOD_color_t color = class_info[actor->class].color;

            if (actor->flash_fade > 0)
            {
                color = TCOD_color_lerp(color, actor->flash_color, actor->flash_fade);
            }

            TCOD_console_set_char_foreground(console, actor->x - view_x, actor->y - view_y, color);
            TCOD_console_set_char(console, actor->x - view_x, actor->y - view_y, race_info[actor->race].glyph);
        }
    }

    if (targeting != TARGETING_NONE)
    {
        TCOD_console_set_char_foreground(console, target_x - view_x, target_y - view_y, TCOD_red);
        TCOD_console_set_char(console, target_x - view_x, target_y - view_y, 'X');

        struct tile *tile = &map->tiles[target_x][target_y];

        if (TCOD_map_is_in_fov(game->player->fov, target_x, target_y))
        {
            if (tile->actor)
            {
                TCOD_console_printf_ex(console, view_width / 2, view_height - 2, TCOD_BKGND_NONE, TCOD_CENTER, tile->actor->name);

                goto done;
            }

            {
                struct item *item = TCOD_list_peek(tile->items);

                if (item)
                {
                    TCOD_console_printf_ex(console, view_width / 2, view_height - 2, TCOD_BKGND_NONE, TCOD_CENTER, item_info[item->type].name);

                    goto done;
                }
            }

            if (tile->object)
            {
                TCOD_console_printf_ex(console, view_width / 2, view_height - 2, TCOD_BKGND_NONE, TCOD_CENTER, object_info[tile->object->type].name);

                goto done;
            }

            TCOD_console_printf_ex(console, view_width / 2, view_height - 2, TCOD_BKGND_NONE, TCOD_CENTER, tile_info[tile->type].name);

        done:;
        }
        else
        {
            if (tile->seen)
            {
                TCOD_console_printf_ex(console, view_width / 2, view_height - 2, TCOD_BKGND_NONE, TCOD_CENTER, "%s (known)", tile_info[tile->type].name);
            }
            else
            {
                TCOD_console_printf_ex(console, view_width / 2, view_height - 2, TCOD_BKGND_NONE, TCOD_CENTER, "Unknown");
            }
        }
    }

    if (message_log_visible)
    {
        TCOD_console_set_default_background(message_log, TCOD_black);
        TCOD_console_set_default_foreground(message_log, TCOD_white);
        TCOD_console_clear(message_log);

        int y = 1;
        TCOD_LIST_FOREACH(game->messages)
        {
            struct message *message = *iterator;

            TCOD_console_set_default_foreground(message_log, message->color);
            TCOD_console_printf(message_log, 1, y, message->text);

            y++;
        }

        TCOD_console_set_default_foreground(message_log, TCOD_white);
        TCOD_console_printf_frame(message_log, 0, 0, message_log_width, message_log_height, false, TCOD_BKGND_SET, "Log");

        TCOD_console_blit(message_log, 0, 0, message_log_width, message_log_height, console, message_log_x, message_log_y, 1, 1);
    }

    if (panel_visible)
    {
        TCOD_console_set_default_background(panel, TCOD_black);
        TCOD_console_set_default_foreground(panel, TCOD_white);
        TCOD_console_clear(panel);

        struct panel_status *current_panel_status = &panel_status[current_panel];

        switch (current_panel)
        {
        case PANEL_CHARACTER:
        {
            int y = 1;
            TCOD_console_printf(panel, 1, y++ - current_panel_status->scroll, "NAME     : %s", game->player->name);
            TCOD_console_printf(panel, 1, y++ - current_panel_status->scroll, "ALIGNMENT: Neutral Good");
            TCOD_console_printf(panel, 1, y++ - current_panel_status->scroll, "RACE     : %s", race_info[game->player->race].name);
            TCOD_console_printf(panel, 1, y++ - current_panel_status->scroll, "CLASS    : %s", class_info[game->player->class].name);
            TCOD_console_printf(panel, 1, y++ - current_panel_status->scroll, "LEVEL    : %d", game->player->level);
            TCOD_console_printf(panel, 1, y++ - current_panel_status->scroll, "EXP      : %d", game->player->experience);
            y++;
            for (enum ability ability = 0; ability < NUM_ABILITIES; ability++)
            {
                TCOD_console_printf(panel, 1, y++ - current_panel_status->scroll, "%s: %d", ability_info[ability].abbreviation, game->player->ability_scores[ability]);
            }
            y++;
            for (enum equip_slot equip_slot = EQUIP_SLOT_ARMOR; equip_slot < NUM_EQUIP_SLOTS; equip_slot++)
            {
                TCOD_console_set_default_foreground(panel, equip_slot == panel_character_get_selected() ? TCOD_yellow : TCOD_white);

                if (game->player->equipment[equip_slot])
                {
                    if (current_panel_status->selection_mode)
                    {
                        TCOD_console_printf(panel, 1, y++ - current_panel_status->scroll, "%c) %s: %s", equip_slot + 'a' - 1, equip_slot_info[equip_slot].label, item_info[game->player->equipment[equip_slot]->type].name);
                    }
                    else
                    {
                        TCOD_console_printf(panel, 1, y++ - current_panel_status->scroll, "%s: %s", equip_slot_info[equip_slot].label, item_info[game->player->equipment[equip_slot]->type].name);
                    }
                }
                else
                {
                    if (current_panel_status->selection_mode)
                    {
                        TCOD_console_printf(panel, 1, y++ - current_panel_status->scroll, "%c) %s: N/A", equip_slot + 'a' - 1, equip_slot_info[equip_slot].label);
                    }
                    else
                    {
                        TCOD_console_printf(panel, 1, y++ - current_panel_status->scroll, "%s: N/A", equip_slot_info[equip_slot].label);
                    }
                }

                TCOD_console_set_default_foreground(panel, TCOD_white);
            }
            y++;
            TCOD_console_printf(panel, 1, y++ - current_panel_status->scroll, "AC: %d", actor_calc_armor_class(game->player));
            TCOD_console_printf(panel, 1, y++ - current_panel_status->scroll, "HP: %d / %d", game->player->current_hp, actor_calc_max_hp(game->player));
            y++;
            int num_dice;
            int die_to_roll;
            int crit_threat;
            int crit_mult;
            actor_calc_weapon(game->player, &num_dice, &die_to_roll, &crit_threat, &crit_mult, false);
            TCOD_console_printf(panel, 1, y++ - current_panel_status->scroll, "MELEE: %dd%d (%d-20x%d)", num_dice, die_to_roll, crit_threat, crit_mult);
            actor_calc_weapon(game->player, &num_dice, &die_to_roll, &crit_threat, &crit_mult, true);
            TCOD_console_printf(panel, 1, y++ - current_panel_status->scroll, "RANGED: %dd%d (%d-20x%d)", num_dice, die_to_roll, crit_threat, crit_mult);
            y++;
            TCOD_console_printf(panel, 1, y++ - current_panel_status->scroll, "ATTACK: +%d", actor_calc_attack_bonus(game->player));
            TCOD_console_printf(panel, 1, y++ - current_panel_status->scroll, "DAMAGE: +%d", actor_calc_damage_bonus(game->player));

            TCOD_console_set_default_foreground(panel, TCOD_white);
            TCOD_console_printf_frame(panel, 0, 0, panel_width, panel_height, false, TCOD_BKGND_SET, "Character");
        }
        break;
        case PANEL_EXAMINE:
        {
            TCOD_console_set_default_foreground(panel, TCOD_white);
            TCOD_console_printf_frame(panel, 0, 0, panel_width, panel_height, false, TCOD_BKGND_SET, "Examine");
        }
        break;
        case PANEL_INVENTORY:
        {
            int y = 1;
            TCOD_LIST_FOREACH(game->player->items)
            {
                struct item *item = *iterator;

                TCOD_console_set_default_foreground(panel, item == panel_inventory_get_selected() ? TCOD_yellow : base_item_info[item_info[item->type].base_item].color);

                if (current_panel_status->selection_mode)
                {
                    TCOD_console_printf(panel, 1, y - current_panel_status->scroll, "%c) %s", y - 1 + 'a' - current_panel_status->scroll, item_info[item->type].name);
                }
                else
                {
                    TCOD_console_printf(panel, 1, y - current_panel_status->scroll, item_info[item->type].name);
                }

                TCOD_console_set_default_foreground(panel, TCOD_white);

                y++;
            }
            TCOD_console_printf_frame(panel, 0, 0, panel_width, panel_height, false, TCOD_BKGND_SET, "Inventory");
        }
        break;
        case PANEL_SPELLBOOK:
        {
            for (int y = 1; y <= 26; y++)
            {
                TCOD_console_printf(panel, 1, y - current_panel_status->scroll, "%d) spell", y);
            }

            TCOD_console_set_default_foreground(panel, TCOD_white);
            TCOD_console_printf_frame(panel, 0, 0, panel_width, panel_height, false, TCOD_BKGND_SET, "Spellbook");
        }
        break;
        }

        TCOD_console_blit(panel, 0, 0, panel_width, panel_height, console, panel_x, panel_y, 1, 1);
    }

    if (tooltip_visible)
    {
        TCOD_console_set_default_background(tooltip, TCOD_black);
        TCOD_console_set_default_foreground(tooltip, TCOD_white);
        TCOD_console_clear(tooltip);

        int y = 1;
        TCOD_LIST_FOREACH(tooltip_options)
        {
            struct tooltip_option *option = *iterator;

            TCOD_console_set_default_foreground(tooltip, option == tooltip_get_selected() ? TCOD_yellow : TCOD_white);
            TCOD_console_printf(tooltip, 1, y, option->text);

            y++;
        }

        TCOD_console_set_default_foreground(tooltip, TCOD_white);
        TCOD_console_printf_frame(tooltip, 0, 0, tooltip_width, tooltip_height, false, TCOD_BKGND_SET, "");

        TCOD_console_blit(tooltip, 0, 0, tooltip_width, tooltip_height, console, tooltip_x, tooltip_y, 1, 1);
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

    // DEBUG
    TCOD_console_printf(console, 0, 0, "Turn: %d", game->turn);
    TCOD_console_printf(console, 0, 1, "Floor: %d", game->player->floor);
    TCOD_console_printf(console, 0, 2, "X: %d", game->player->x);
    TCOD_console_printf(console, 0, 3, "Y: %d", game->player->y);
    TCOD_console_printf(console, 0, 4, "HP: %d", game->player->current_hp);
    TCOD_console_printf(console, 0, 5, "Kills: %d", game->player->kills);
}

static void quit(void)
{
    if (game->state != GAME_STATE_LOSE)
    {
        game_save(SAVE_PATH);
    }

    game_quit();

    TCOD_LIST_FOREACH(tooltip_options)
    {
        struct tooltip_option *tooltip_option = *iterator;

        tooltip_option_destroy(tooltip_option);
    }

    TCOD_console_delete(message_log);
    TCOD_console_delete(panel);
    TCOD_console_delete(tooltip);
    TCOD_list_delete(tooltip_options);

    TCOD_noise_delete(noise);
}

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

static void on_hit_set_took_turn(void *on_hit_params)
{
    took_turn = true;
}

static bool toolip_on_click_move(void)
{
    automoving = true;
    automove_x = tooltip_data.x;
    automove_y = tooltip_data.y;

    return false;
}

static bool message_log_is_inside(int x, int y)
{
    return message_log_visible && x >= message_log_x && x < message_log_x + message_log_width && y >= message_log_y && y < message_log_y + message_log_height;
}

static bool panel_is_inside(int x, int y)
{
    return panel_visible && x >= panel_x && x < panel_x + panel_width && y >= panel_y && y < panel_y + panel_height;
}

static void panel_toggle(enum panel panel)
{
    if (panel_visible)
    {
        if (current_panel == panel)
        {
            panel_visible = false;
        }
        else
        {
            current_panel = panel;
        }
    }
    else
    {
        current_panel = panel;
        panel_visible = true;
    }
}

static void panel_show(enum panel panel)
{
    if (!panel_visible || current_panel != panel)
    {
        panel_toggle(panel);
    }
}

static enum equip_slot panel_character_get_selected(void)
{
    if (panel_visible && current_panel == PANEL_CHARACTER)
    {
        int y = 15;
        for (enum equip_slot equip_slot = 1; equip_slot < NUM_EQUIP_SLOTS; equip_slot++)
        {
            if (mouse_x > panel_x && mouse_x < panel_x + (int)strlen(equip_slot_info[equip_slot].label) + 1 + 3 && mouse_y == y + panel_y - panel_status[current_panel].scroll)
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
    if (panel_visible && current_panel == PANEL_INVENTORY)
    {
        int y = 1;
        TCOD_LIST_FOREACH(game->player->items)
        {
            struct item *item = *iterator;

            if (mouse_x > panel_x && mouse_x < panel_x + (int)strlen(item_info[item->type].name) + 1 + 3 && mouse_y == y + panel_y - panel_status[current_panel].scroll)
            {
                return item;
            }

            y++;
        }
    }

    return NULL;
}

static bool tooltip_is_inside(int x, int y)
{
    return tooltip_visible && x >= tooltip_x && x < tooltip_x + tooltip_width && y >= tooltip_y && y < tooltip_y + tooltip_height;
}

static void tooltip_show(void)
{
    tooltip_options_clear();

    tooltip_visible = true;
    tooltip_x = mouse_x;
    tooltip_y = mouse_y;
}

static void tooltip_hide(void)
{
    tooltip_options_clear();

    tooltip_visible = false;
}

struct tooltip_option *tooltip_option_create(char *text, bool(*on_click)(void))
{
    struct tooltip_option *tooltip_option = malloc(sizeof(struct tooltip_option));

    tooltip_option->text = _strdup(text);
    tooltip_option->on_click = on_click;

    return tooltip_option;
}

void tooltip_option_destroy(struct tooltip_option *tooltip_option)
{
    free(tooltip_option->text);
    free(tooltip_option);
}

static void tooltip_options_add(char *text, bool(*on_click)(void))
{
    struct tooltip_option *tooltip_option = tooltip_option_create(text, on_click);

    TCOD_list_push(tooltip_options, tooltip_option);
}

static void tooltip_options_clear(void)
{
    TCOD_LIST_FOREACH(tooltip_options)
    {
        struct tooltip_option *tooltip_option = *iterator;

        iterator = TCOD_list_remove_iterator(tooltip_options, iterator);

        tooltip_option_destroy(tooltip_option);
    }
}

static struct tooltip_option *tooltip_get_selected(void)
{
    if (tooltip_visible)
    {
        int y = 1;
        TCOD_LIST_FOREACH(tooltip_options)
        {
            struct tooltip_option *option = *iterator;

            if (mouse_x > tooltip_x && mouse_x < tooltip_x + (int)strlen(option->text) + 1 && mouse_y == y + tooltip_y)
            {
                return option;
            }

            y++;
        }
    }

    return NULL;
}

static bool view_is_inside(int x, int y)
{
    return x >= 0 && x < view_width && y >= 0 && y < view_height;
}
