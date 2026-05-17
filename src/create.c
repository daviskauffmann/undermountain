#include "create.h"
#include "config.h"
#include "ui.h"
#include "game/actor.h"
#include "game/ability.h"
#include "game/base_attack_bonus.h"
#include "game/class.h"
#include "game/feat.h"
#include "game/base_item.h"
#include "game/list.h"
#include "game/light.h"
#include "game/item.h"
#include "game/faction.h"
#include "game/world.h"
#include "game/monster.h"
#include "game/race.h"
#include "game/size.h"
#include "game/special_ability.h"
#include "game/spell.h"
#include "scene.h"
#include <curses.h>
#include <ctype.h>
#include <math.h>
#include <string.h>
#include <time.h>

extern struct scene menu_scene;
extern struct scene game_scene;

#define STARTING_LEVEL 1

enum create_state
{
    CREATE_STATE_NAME,
    CREATE_STATE_RACE,
    CREATE_STATE_CLASS,
    CREATE_STATE_ABILITY_SCORES,
    CREATE_STATE_FEATS,

    CREATE_NUM_STATES
};

static enum create_state state;

static char name[32];
static bool default_name_modified;
static enum race selected_race;
static enum class selected_class;
static int ability_points;
static int ability_scores[NUM_ABILITIES];
static enum ability selected_ability;
static bool feats[NUM_FEATS];
static int remaining_feats;
static struct list *available_feats;
static size_t selected_feat_index;

#define MAX_NAME_LENGTH 32

static int get_ability_score_cost(const int ability_score)
{
    if (ability_score <= 14)
    {
        return 1;
    }
    else if (ability_score <= 16)
    {
        return 2;
    }
    else if (ability_score <= 18)
    {
        return 3;
    }
    return 4;
}

static void reset_ability_scores(void)
{
    for (enum ability ability = ABILITY_NONE + 1; ability < NUM_ABILITIES; ability++)
    {
        ability_scores[ability] = 8;
    }
    ability_points = NUM_ABILITIES * 5;
}

static void set_default_ability_scores(void)
{
    for (enum ability ability = ABILITY_NONE + 1; ability < NUM_ABILITIES; ability++)
    {
        ability_scores[ability] = class_database[selected_class].default_ability_scores[ability];
    }
    ability_points = 0;
}

static bool feat_is_available(enum feat feat)
{
    const struct feat_prerequisites *const prerequisites = &feat_database[feat].prerequisites;

    if (prerequisites->race != RACE_NONE && prerequisites->race != selected_race)
    {
        return false;
    }

    if (prerequisites->class != CLASS_NONE && prerequisites->class != selected_class)
    {
        return false;
    }

    if (prerequisites->level > STARTING_LEVEL)
    {
        return false;
    }

    const int base_attack_bonus = (int)floorf(STARTING_LEVEL * base_attack_bonus_database[class_database[selected_class].base_attack_bonus_type].multiplier);

    if (prerequisites->base_attack_bonus > base_attack_bonus)
    {
        return false;
    }

    for (enum ability ability = ABILITY_NONE + 1; ability < NUM_ABILITIES; ability++)
    {
        if (prerequisites->ability_scores[ability] > 0 &&
            ability_scores[ability] < prerequisites->ability_scores[ability])
        {
            return false;
        }
    }

    for (enum feat _feat = FEAT_NONE + 1; _feat < NUM_FEATS; _feat++)
    {
        if (prerequisites->feats[_feat] && !feats[_feat])
        {
            return false;
        }
    }

    return true;
}

static void recalculate_available_feats(void)
{
    list_clear(available_feats);

    for (enum feat feat = FEAT_NONE + 1; feat < NUM_FEATS; feat++)
    {
        if (feat_is_available(feat))
        {
            list_add(available_feats, (void *)(long long)feat);
        }
    }
}

static void reset_feats(void)
{
    for (enum feat feat = FEAT_NONE + 1; feat < NUM_FEATS; feat++)
    {
        feats[feat] = false;
    }

    remaining_feats = 1;

    if (race_database[selected_race].feats[FEAT_QUICK_TO_MASTER])
    {
        remaining_feats++;
    }

    if (selected_class == CLASS_FIGHTER)
    {
        remaining_feats++;
    }

    recalculate_available_feats();
}

static void init(const struct scene *const previous_scene)
{
    (void)previous_scene;
    state = CREATE_STATE_NAME;
    strcpy(name, "Adventurer");
    default_name_modified = false;
    selected_race = RACE_HUMAN;
    selected_class = CLASS_FIGHTER;
    set_default_ability_scores();
    selected_ability = ABILITY_NONE + 1;
    available_feats = list_new();
    reset_feats();
    selected_feat_index = 0;
}

static void uninit(void)
{
    if (available_feats)
    {
        list_delete(available_feats);
        available_feats = NULL;
    }
}

static bool handle_name_input(int ch, struct scene **next_scene)
{
    (void)next_scene;

    if (ch == KEY_BACKSPACE || ch == 127 || ch == '\b')
    {
        if (default_name_modified)
        {
            const int length = (int)strlen(name);
            if (length > 0)
            {
                name[length - 1] = '\0';
            }
        }
        else
        {
            default_name_modified = true;
            name[0] = '\0';
        }
        return true;
    }
    else if (ch == '\n' || ch == '\r' || ch == KEY_ENTER)
    {
        state = CREATE_STATE_RACE;
        return true;
    }
    else if (ch == 27) // ESC
    {
        *next_scene = &menu_scene;
        return true;
    }
    else if (isalpha(ch))
    {
        const int length = (int)strlen(name);
        if (length < (int)(sizeof(name) - 1))
        {
            name[length] = (char)ch;
            name[length + 1] = '\0';
            default_name_modified = true;
        }
        return true;
    }
    return false;
}

static bool handle_race_input(int ch, struct scene **next_scene)
{
    (void)next_scene;

    if (ch == KEY_UP || ch == 'k' || ch == 'K')
    {
        selected_race = (selected_race == PLAYER_RACE_BEGIN) ? PLAYER_RACE_END : (selected_race - 1);
        set_default_ability_scores();
        reset_feats();
        return true;
    }
    else if (ch == KEY_DOWN || ch == 'j' || ch == 'J')
    {
        selected_race = (selected_race == PLAYER_RACE_END) ? PLAYER_RACE_BEGIN : (selected_race + 1);
        set_default_ability_scores();
        reset_feats();
        return true;
    }
    else if (ch == '\n' || ch == '\r' || ch == KEY_ENTER)
    {
        state = CREATE_STATE_CLASS;
        return true;
    }
    else if (ch == 27) // ESC
    {
        state = CREATE_STATE_NAME;
        return true;
    }
    return false;
}

static bool handle_class_input(int ch, struct scene **next_scene)
{
    (void)next_scene;

    if (ch == KEY_UP || ch == 'k' || ch == 'K')
    {
        selected_class = (selected_class == PLAYER_CLASS_BEGIN) ? PLAYER_CLASS_END : (selected_class - 1);
        set_default_ability_scores();
        reset_feats();
        return true;
    }
    else if (ch == KEY_DOWN || ch == 'j' || ch == 'J')
    {
        selected_class = (selected_class == PLAYER_CLASS_END) ? PLAYER_CLASS_BEGIN : (selected_class + 1);
        set_default_ability_scores();
        reset_feats();
        return true;
    }
    else if (ch == '\n' || ch == '\r' || ch == KEY_ENTER)
    {
        state = CREATE_STATE_ABILITY_SCORES;
        return true;
    }
    else if (ch == 27) // ESC
    {
        state = CREATE_STATE_RACE;
        return true;
    }
    return false;
}

static bool handle_ability_scores_input(int ch, struct scene **next_scene)
{
    (void)next_scene;

    if (ch == KEY_UP || ch == 'k' || ch == 'K')
    {
        selected_ability = (selected_ability == ABILITY_NONE + 1) ? (NUM_ABILITIES - 1) : (selected_ability - 1);
        return true;
    }
    else if (ch == KEY_DOWN || ch == 'j' || ch == 'J')
    {
        selected_ability = (selected_ability == NUM_ABILITIES - 1) ? (ABILITY_NONE + 1) : (selected_ability + 1);
        return true;
    }
    else if (ch == KEY_LEFT || ch == 'h' || ch == 'H')
    {
        const int cost = get_ability_score_cost(ability_scores[selected_ability]);
        if (ability_scores[selected_ability] > 8)
        {
            ability_scores[selected_ability]--;
            ability_points += cost;
        }
        return true;
    }
    else if (ch == KEY_RIGHT || ch == 'l' || ch == 'L')
    {
        const int cost = get_ability_score_cost(ability_scores[selected_ability] + 1);
        if (ability_points >= cost && ability_scores[selected_ability] < 18)
        {
            ability_scores[selected_ability]++;
            ability_points -= cost;
        }
        return true;
    }
    else if (ch == 'r' || ch == 'R')
    {
        set_default_ability_scores();
        return true;
    }
    else if (ch == '\n' || ch == '\r' || ch == KEY_ENTER)
    {
        state = CREATE_STATE_FEATS;
        return true;
    }
    else if (ch == 27) // ESC
    {
        state = CREATE_STATE_CLASS;
        return true;
    }
    return false;
}

static bool handle_feats_input(int ch, struct scene **next_scene)
{
    (void)next_scene;

    if (ch == KEY_UP || ch == 'k' || ch == 'K')
    {
        if (available_feats->size > 0)
        {
            selected_feat_index = (selected_feat_index == 0) ? (available_feats->size - 1) : (selected_feat_index - 1);
        }
        return true;
    }
    else if (ch == KEY_DOWN || ch == 'j' || ch == 'J')
    {
        if (available_feats->size > 0)
        {
            selected_feat_index = (selected_feat_index == available_feats->size - 1) ? 0 : (selected_feat_index + 1);
        }
        return true;
    }
    else if ((ch == 'x' || ch == 'X' || ch == ' ') && available_feats->size > 0)
    {
        const enum feat selected_feat = (enum feat)(uintptr_t)list_get(available_feats, selected_feat_index);

        // Can't toggle racial or class feats
        if (!race_database[selected_race].feats[selected_feat] &&
            class_database[selected_class].feat_progression[selected_feat] == 0)
        {
            if (feats[selected_feat])
            {
                feats[selected_feat] = false;
                remaining_feats++;
                recalculate_available_feats();
            }
            else if (remaining_feats > 0)
            {
                feats[selected_feat] = true;
                remaining_feats--;
                recalculate_available_feats();
            }
        }
        return true;
    }
    else if (ch == 'r' || ch == 'R')
    {
        reset_feats();
        return true;
    }
    else if (ch == '\n' || ch == '\r' || ch == KEY_ENTER)
    {
        // Character creation complete - initialize world and hero
        world_init();

        struct actor *const hero = actor_new(
            name,
            selected_race,
            selected_class,
            race_database[selected_race].size,
            FACTION_ADVENTURER,
            STARTING_LEVEL,
            ability_scores,
            (bool[NUM_SPECIAL_ABILITIES]){false},
            (bool[NUM_SPECIAL_ATTACKS]){false},
            feats,
            0,
            0, 0);

        for (enum equip_slot equip_slot = EQUIP_SLOT_NONE + 1; equip_slot < NUM_EQUIP_SLOTS; equip_slot++)
        {
            const enum item_type item_type = class_database[selected_class].starting_equipment[equip_slot];

            if (item_type != ITEM_TYPE_NONE)
            {
                hero->equipment[equip_slot] = item_new(item_type, 0, 0, 0, base_item_database[item_database[item_type].type].max_stack);
            }
        }

        for (enum item_type item_type = ITEM_TYPE_NONE + 1; item_type < NUM_ITEM_TYPES; item_type++)
        {
            const int stack = class_database[selected_class].starting_items[item_type];

            if (stack > 0)
            {
                list_add(hero->items, item_new(item_type, 0, 0, 0, stack));
            }
        }

        hero->light_type = LIGHT_TYPE_PLAYER;

        world_create(hero, (unsigned int)time(0)); // TODO: user input seed

        *next_scene = &game_scene;

        return true;
    }
    else if (ch == 27) // ESC
    {
        state = CREATE_STATE_ABILITY_SCORES;
        return true;
    }
    return false;
}

static struct scene *handle_input(void)
{
    int ch = getch();
    if (ch == ERR)
    {
        return &create_scene;
    }

    struct scene *next_scene = &create_scene;

    switch (state)
    {
    case CREATE_STATE_NAME:
        handle_name_input(ch, &next_scene);
        break;
    case CREATE_STATE_RACE:
        handle_race_input(ch, &next_scene);
        break;
    case CREATE_STATE_CLASS:
        handle_class_input(ch, &next_scene);
        break;
    case CREATE_STATE_ABILITY_SCORES:
        handle_ability_scores_input(ch, &next_scene);
        break;
    case CREATE_STATE_FEATS:
        handle_feats_input(ch, &next_scene);
        break;
    default:
        break;
    }

    return next_scene;
}

static void render(void)
{
    int max_y, max_x;
    getmaxyx(stdscr, max_y, max_x);

    ui_screen_frame(TITLE);

    // State-specific rendering
    int y = 2;

    switch (state)
    {
    case CREATE_STATE_NAME:
    {
        ui_footer_hint("[Enter] next  [Backspace] delete  [ESC] back");
        char welcome_text[128];
        snprintf(welcome_text, sizeof(welcome_text),
                 "Welcome to %s. You kill stuff. The end.", TITLE);
        ui_centered(y++, welcome_text, 0);
        y++;

        mvprintw(y++, 2, "What is your name?");
        y++;
        attron(A_REVERSE);
        mvprintw(y, 2, "%-30s", name);
        attroff(A_REVERSE);
        break;
    }

    case CREATE_STATE_RACE:
        ui_footer_hint("[↑↓/jk] select  [Enter] next  [ESC] back");
        mvprintw(y++, 2, "What is your race?");
        y++;

        for (enum race race = PLAYER_RACE_BEGIN; race <= PLAYER_RACE_END; race++)
        {
            ui_list_item(y++, 2, race_database[race].name, race == selected_race);
        }

        y++;
        mvprintw(y++, 2, "Size: %s", size_database[race_database[selected_race].size].name);
        y++;

        mvprintw(y++, 2, "Ability adjustments:");
        for (enum ability ability = ABILITY_NONE + 1; ability < NUM_ABILITIES; ability++)
        {
            const int adj = race_database[selected_race].ability_adjustments[ability];
            if (adj != 0)
            {
                mvprintw(y++, 2, "  %s: %+d", ability_database[ability].name, adj);
            }
        }
        break;

    case CREATE_STATE_CLASS:
        ui_footer_hint("[↑↓/jk] select  [Enter] next  [ESC] back");
        mvprintw(y++, 2, "What is your class?");
        y++;

        for (enum class class = PLAYER_CLASS_BEGIN; class <= PLAYER_CLASS_END; class++)
        {
            ui_list_item(y++, 2, class_database[class].name, class == selected_class);
        }

        y++;
        mvprintw(y++, 2, "Hit die: %s", class_database[selected_class].hit_die);
        mvprintw(y++, 2, "Base attack bonus: %s (level x%.2f)",
                 base_attack_bonus_database[class_database[selected_class].base_attack_bonus_type].name,
                 base_attack_bonus_database[class_database[selected_class].base_attack_bonus_type].multiplier);
        break;

    case CREATE_STATE_ABILITY_SCORES:
        ui_footer_hint("[↑↓/jk] select  [←→/hl] adjust  [R] recommended  [Enter] next  [ESC] back");
        mvprintw(y++, 2, "What are your abilities?");
        y++;

        for (enum ability ability = ABILITY_NONE + 1; ability < NUM_ABILITIES; ability++)
        {
            const int base_score = ability_scores[ability];
            const int adjusted_score = base_score + race_database[selected_race].ability_adjustments[ability];
            const int modifier = (int)floorf((adjusted_score - 10) / 2.0f);
            const int cost = get_ability_score_cost(base_score + 1);

            char buffer[64];
            snprintf(buffer, sizeof(buffer), "%s: %2d (%+d) (cost: %d)",
                     ability_database[ability].name,
                     adjusted_score,
                     modifier,
                     cost);
            ui_list_item(y++, 2, buffer, ability == selected_ability);
        }

        y++;
        mvprintw(y++, 2, "Remaining Points: %d", ability_points);
        break;

    case CREATE_STATE_FEATS:
        ui_footer_hint("[↑↓/jk] select  [X/Space] toggle  [R] reset  [Enter] done  [ESC] back");
        mvprintw(y++, 2, "What are your feats?");
        y++;

        if (available_feats->size == 0)
        {
            mvprintw(y++, 2, "(No feats available)");
        }
        else
        {
            const enum feat selected_feat = (enum feat)(uintptr_t)list_get(available_feats, selected_feat_index);

            for (size_t i = 0; i < available_feats->size && y < max_y - 4; i++)
            {
                const enum feat feat = (enum feat)(uintptr_t)list_get(available_feats, i);
                const char *marker = "[ ]";
                char buffer[128];

                if (race_database[selected_race].feats[feat])
                {
                    marker = "[-]";
                    snprintf(buffer, sizeof(buffer), "%s %s (racial)", marker, feat_database[feat].name);
                }
                else if (class_database[selected_class].feat_progression[feat] == 1)
                {
                    marker = "[-]";
                    snprintf(buffer, sizeof(buffer), "%s %s (class)", marker, feat_database[feat].name);
                }
                else if (feats[feat])
                {
                    marker = "[x]";
                    snprintf(buffer, sizeof(buffer), "%s %s", marker, feat_database[feat].name);
                }
                else
                {
                    snprintf(buffer, sizeof(buffer), "%s %s", marker, feat_database[feat].name);
                }

                ui_list_item(y++, 2, buffer, feat == selected_feat);
            }
        }

        y++;
        mvprintw(y++, 2, "Remaining Feats: %d", remaining_feats);
        break;
    }

    refresh();
}

static struct scene *update(const float delta_time)
{
    (void)delta_time;

    struct scene *next_scene = handle_input();

    if (next_scene != &create_scene)
    {
        return next_scene;
    }

    render();
    return &create_scene;
}

struct scene create_scene = {
    &init,
    &uninit,
    &update};
