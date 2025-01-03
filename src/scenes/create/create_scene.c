#include "create_scene.h"

#include "../../config.h"
#include "../../game/actor.h"
#include "../../game/base_attack_bonus.h"
#include "../../game/base_item.h"
#include "../../game/class.h"
#include "../../game/color.h"
#include "../../game/faction.h"
#include "../../game/feat.h"
#include "../../game/light.h"
#include "../../game/list.h"
#include "../../game/monster.h"
#include "../../game/race.h"
#include "../../game/world.h"
#include "../../print.h"
#include "../../scene.h"
#include "../game/game_scene.h"
#include "../menu/menu_scene.h"
#include "state.h"
#include <libtcod.h>
#include <time.h>

#define STARTING_LEVEL 1

static enum state state;

static char name[32];
static bool default_name_modified;

static enum race selected_race;

static enum class selected_class;

static int ability_points;
static int ability_scores[NUM_ABILITIES];
static enum ability selected_ability;

static bool feats[NUM_FEATS];
static int remaining_feats;
struct list *available_feats;
static size_t selected_feat_index;

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

    if (prerequisites->race != RACE_NONE &&
        prerequisites->race != selected_race)
    {
        return false;
    }

    if (prerequisites->class != CLASS_NONE &&
        prerequisites->class != selected_class)
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
        if (prerequisites->feats[_feat] &&
            !feats[_feat])
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
    previous_scene;

    state = STATE_STORY;

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
    list_delete(available_feats);
}

static struct scene *handle_event(const SDL_Event *const event)
{
    switch (event->type)
    {
    case SDL_KEYDOWN:
    {
        switch (event->key.keysym.sym)
        {
        case SDLK_BACKSPACE:
        {
            switch (state)
            {
            case STATE_NAME:
            {
                if (default_name_modified)
                {
                    const size_t length = strlen(name);

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
            }
            break;
            }
        }
        break;
        case SDLK_RETURN:
        {
            state++;

            if (state == NUM_STATES)
            {
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

                create_scene.uninit();
                game_scene.init(&create_scene);
                return &game_scene;
            }
        }
        break;
        case SDLK_ESCAPE:
        {
            state--;

            if (state < 0)
            {
                create_scene.uninit();
                menu_scene.init(&create_scene);
                return &menu_scene;
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
            const int alpha = event->key.keysym.sym - SDLK_a;

            switch (state)
            {
            case STATE_NAME:
            {
                const size_t length = strlen(name);

                if (length < sizeof(name) - 1)
                {
                    name[length] = (char)(alpha + (event->key.keysym.mod & KMOD_SHIFT ? 'A' : 'a'));
                    name[length + 1] = '\0';

                    default_name_modified = true;
                }
            }
            break;
            case STATE_ABILITY_SCORES:
            {
                if (event->key.keysym.sym == SDLK_r)
                {
                    set_default_ability_scores();
                }
            }
            break;
            case STATE_FEATS:
            {
                if (event->key.keysym.sym == SDLK_r)
                {
                    reset_feats();
                }

                if (event->key.keysym.sym == SDLK_x)
                {
                    const enum feat selected_feat = (enum feat)(uintptr_t)list_get(available_feats, selected_feat_index);

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
                }
            }
            break;
            }
        }
        break;
        case SDLK_KP_2:
        case SDLK_DOWN:
        {
            switch (state)
            {
            case STATE_RACE:
            {
                if (selected_race == PLAYER_RACE_END)
                {
                    selected_race = PLAYER_RACE_BEGIN;
                }
                else
                {
                    selected_race++;
                }

                set_default_ability_scores();
                reset_feats();
            }
            break;
            case STATE_CLASS:
            {
                if (selected_class == PLAYER_CLASS_END)
                {
                    selected_class = PLAYER_CLASS_BEGIN;
                }
                else
                {
                    selected_class++;
                }

                set_default_ability_scores();
                reset_feats();
            }
            break;
            case STATE_ABILITY_SCORES:
            {
                if (selected_ability == NUM_ABILITIES - 1)
                {
                    selected_ability = ABILITY_NONE + 1;
                }
                else
                {
                    selected_ability++;
                }
            }
            break;
            case STATE_FEATS:
            {
                if (selected_feat_index == available_feats->size - 1)
                {
                    selected_feat_index = 0;
                }
                else
                {
                    selected_feat_index++;
                }
            }
            break;
            }
        }
        break;
        case SDLK_KP_4:
        case SDLK_LEFT:
        {
            switch (state)
            {
            case STATE_ABILITY_SCORES:
            {
                const int cost = get_ability_score_cost(ability_scores[selected_ability]);

                if (ability_scores[selected_ability] > 8)
                {
                    ability_scores[selected_ability]--;
                    ability_points += cost;
                }
            }
            break;
            }
        }
        break;
        case SDLK_KP_6:
        case SDLK_RIGHT:
        {
            switch (state)
            {
            case STATE_ABILITY_SCORES:
            {
                const int cost = get_ability_score_cost(ability_scores[selected_ability] + 1);

                if (ability_points >= cost && ability_scores[selected_ability] < 18)
                {
                    ability_scores[selected_ability]++;
                    ability_points -= cost;
                }
            }
            break;
            }
        }
        break;
        case SDLK_KP_8:
        case SDLK_UP:
        {
            switch (state)
            {
            case STATE_RACE:
            {
                if (selected_race == PLAYER_RACE_BEGIN)
                {
                    selected_race = PLAYER_RACE_END;
                }
                else
                {
                    selected_race--;
                }

                set_default_ability_scores();
                reset_feats();
            }
            break;
            case STATE_CLASS:
            {
                if (selected_class == PLAYER_CLASS_BEGIN)
                {
                    selected_class = PLAYER_CLASS_END;
                }
                else
                {
                    selected_class--;
                }

                set_default_ability_scores();
                reset_feats();
            }
            break;
            case STATE_ABILITY_SCORES:
            {
                if (selected_ability == ABILITY_NONE + 1)
                {
                    selected_ability = NUM_ABILITIES - 1;
                }
                else
                {
                    selected_ability--;
                }
            }
            break;
            case STATE_FEATS:
            {
                if (selected_feat_index == 0)
                {
                    selected_feat_index = available_feats->size - 1;
                }
                else
                {
                    selected_feat_index--;
                }
            }
            break;
            }
        }
        break;
        }
    }
    break;
    }

    return &create_scene;
}

static struct scene *update(TCOD_Console *const console, const float delta_time)
{
    delta_time;

    console_print(
        console,
        1, console_height - 2,
        &color_white,
        &color_black,
        TCOD_BKGND_NONE,
        TCOD_LEFT,
        "<- ESC");

    console_print(
        console,
        console_width - 2, console_height - 2,
        &color_white,
        &color_black,
        TCOD_BKGND_NONE,
        TCOD_RIGHT,
        "ENTER ->");

    switch (state)
    {
    case STATE_STORY:
    {
        console_print(
            console,
            console_width / 2, 2,
            &color_white,
            &color_black,
            TCOD_BKGND_NONE,
            TCOD_CENTER,
            "Welcome to Undermountain. You kill stuff. The end.");
    }
    break;
    case STATE_NAME:
    {
        console_print(
            console,
            1, 1,
            &color_white,
            &color_black,
            TCOD_BKGND_NONE,
            TCOD_LEFT,
            "What is your name?");

        TCOD_console_printn_frame(
            console,
            1, 2,
            20, 3,
            0,
            NULL,
            &color_white,
            &color_black,
            TCOD_BKGND_NONE,
            false);
        TCOD_console_printf(
            console,
            2, 3,
            "%s",
            name);
    }
    break;
    case STATE_RACE:
    {
        console_print(
            console,
            1, 1,
            &color_white,
            &color_black,
            TCOD_BKGND_NONE,
            TCOD_LEFT,
            "What is your race?");

        int y = 2;

        for (enum race race = PLAYER_RACE_BEGIN; race <= PLAYER_RACE_END; race++)
        {
            console_print(
                console,
                1, y++,
                race == selected_race ? &color_yellow : &color_white,
                &color_black,
                TCOD_BKGND_NONE,
                TCOD_LEFT,
                "%s",
                race_database[race].name);
        }

        y++;

        console_print(
            console,
            1, y++,
            &color_white,
            &color_black,
            TCOD_BKGND_NONE,
            TCOD_LEFT,
            "Size: %s",
            size_database[race_database[selected_race].size].name);

        y++;

        console_print(
            console,
            1, y++,
            &color_white,
            &color_black,
            TCOD_BKGND_NONE,
            TCOD_LEFT,
            "Ability adjustments:");
        for (enum ability ability = ABILITY_NONE + 1; ability < NUM_ABILITIES; ability++)
        {
            const int ability_adjustment = race_database[selected_race].ability_adjustments[ability];

            if (ability_adjustment != 0)
            {
                console_print(
                    console,
                    1, y++,
                    &color_white,
                    &color_black,
                    TCOD_BKGND_NONE,
                    TCOD_LEFT,
                    "- %s: %d",
                    ability_database[ability].name,
                    ability_adjustment);
            }
        }

        y++;

        console_print(
            console,
            1, y++,
            &color_white,
            &color_black,
            TCOD_BKGND_NONE,
            TCOD_LEFT,
            "Special abilities:");
        for (enum special_ability special_ability = 0; special_ability < NUM_SPECIAL_ABILITIES; special_ability++)
        {
            if (race_database[selected_race].special_abilities[special_ability] != SPECIAL_ABILITY_NONE)
            {
                console_print(
                    console,
                    1, y++,
                    &color_white,
                    &color_black,
                    TCOD_BKGND_NONE,
                    TCOD_LEFT,
                    "- %s",
                    special_ability_database[special_ability].name);
            }
        }

        y++;

        console_print(
            console,
            1, y++,
            &color_white,
            &color_black,
            TCOD_BKGND_NONE,
            TCOD_LEFT,
            "Feats:");
        for (enum feat feat = FEAT_NONE + 1; feat < NUM_FEATS; feat++)
        {
            if (race_database[selected_race].feats[feat] != FEAT_NONE)
            {
                console_print(
                    console,
                    1, y++,
                    &color_white,
                    &color_black,
                    TCOD_BKGND_NONE,
                    TCOD_LEFT,
                    "- %s",
                    feat_database[feat].name);
            }
        }
    }
    break;
    case STATE_CLASS:
    {
        console_print(
            console,
            1, 1,
            &color_white,
            &color_black,
            TCOD_BKGND_NONE,
            TCOD_LEFT,
            "What is your class?");

        int y = 2;

        for (enum class class = PLAYER_CLASS_BEGIN; class <= PLAYER_CLASS_END; class ++)
        {
            console_print(
                console,
                1, y++,
                class == selected_class ? &color_yellow : &color_white,
                &color_black,
                TCOD_BKGND_NONE,
                TCOD_LEFT,
                "%s",
                class_database[class].name);
        }

        y++;

        console_print(
            console,
            1, y++,
            &color_white,
            &color_black,
            TCOD_BKGND_NONE,
            TCOD_LEFT,
            "Hit die: %s",
            class_database[selected_class].hit_die);

        console_print(
            console,
            1, y++,
            &color_white,
            &color_black,
            TCOD_BKGND_NONE,
            TCOD_LEFT,
            "Base attack bonus: %s (level x%.2f)",
            base_attack_bonus_database[class_database[selected_class].base_attack_bonus_type].name,
            base_attack_bonus_database[class_database[selected_class].base_attack_bonus_type].multiplier);

        y++;

        console_print(
            console,
            1, y++,
            &color_white,
            &color_black,
            TCOD_BKGND_NONE,
            TCOD_LEFT,
            "Feats:");
        for (enum feat feat = FEAT_NONE + 1; feat < NUM_FEATS; feat++)
        {
            if (class_database[selected_class].feat_progression[feat])
            {
                console_print(
                    console,
                    1,
                    y++,
                    &color_white,
                    &color_black,
                    TCOD_BKGND_NONE,
                    TCOD_LEFT,
                    "- %s",
                    feat_database[feat].name);
            }
        }

        y++;

        if (class_database[selected_class].spellcasting_ability != ABILITY_NONE)
        {
            console_print(
                console,
                1, y++,
                &color_white,
                &color_black,
                TCOD_BKGND_NONE,
                TCOD_LEFT,
                "Spellcasting ability: %s",
                ability_database[class_database[selected_class].spellcasting_ability].name);

            console_print(
                console,
                1, y++,
                &color_white,
                &color_black,
                TCOD_BKGND_NONE,
                TCOD_LEFT,
                "Starting spells:");
            for (enum spell_type spell_type = SPELL_TYPE_NONE + 1; spell_type < NUM_SPELL_TYPES; spell_type++)
            {
                if (class_database[selected_class].spell_progression[spell_type] == 1)
                {
                    console_print(
                        console,
                        1,
                        y++,
                        &color_white,
                        &color_black,
                        TCOD_BKGND_NONE,
                        TCOD_LEFT,
                        "- %s",
                        spell_database[spell_type].name);
                }
            }
        }

        y++;

        console_print(
            console,
            1, y++,
            &color_white,
            &color_black,
            TCOD_BKGND_NONE,
            TCOD_LEFT,
            "Companion: %s",
            monster_database[class_database[selected_class].companion].name);
    }
    break;
    case STATE_ABILITY_SCORES:
    {
        console_print(
            console,
            1, 1,
            &color_white,
            &color_black,
            TCOD_BKGND_NONE,
            TCOD_LEFT,
            "What are your abilities?");

        console_print(
            console,
            1, console_height - 4,
            &color_white,
            &color_black,
            TCOD_BKGND_NONE,
            TCOD_LEFT,
            "right) Increase, left) Decrease, r) Recommended");

        int y = 2;

        for (enum ability ability = ABILITY_NONE + 1; ability < NUM_ABILITIES; ability++)
        {
            const int base_score = ability_scores[ability];
            const int adjusted_score = base_score + race_database[selected_race].ability_adjustments[ability];
            const int modifier = (int)floorf((adjusted_score - 10) / 2.0f);
            const int cost = get_ability_score_cost(base_score + 1);

            console_print(
                console,
                1, y++,
                ability == selected_ability ? &color_yellow : &color_white,
                &color_black,
                TCOD_BKGND_NONE,
                TCOD_LEFT,
                "%s: %d (%d) (cost: %d)",
                ability_database[ability].name,
                adjusted_score,
                modifier,
                cost);
        }

        y++;

        console_print(
            console,
            1, y++,
            &color_white,
            &color_black,
            TCOD_BKGND_NONE,
            TCOD_LEFT,
            "Remaining Points: %d",
            ability_points);

        y++;

        console_print(
            console,
            1, y++,
            &color_white,
            &color_black,
            TCOD_BKGND_NONE,
            TCOD_LEFT,
            "%s",
            ability_database[selected_ability].description);
    }
    break;
    case STATE_FEATS:
    {
        console_print(
            console,
            1, 1,
            &color_white,
            &color_black,
            TCOD_BKGND_NONE,
            TCOD_LEFT,
            "What are your feats?");

        console_print(
            console,
            1, console_height - 4,
            &color_white,
            &color_black,
            TCOD_BKGND_NONE,
            TCOD_LEFT,
            "x) Select, r) Recommended");

        int y = 2;

        const enum feat selected_feat = (enum feat)(uintptr_t)list_get(available_feats, selected_feat_index);

        for (size_t available_feat_index = 0; available_feat_index < available_feats->size; available_feat_index++)
        {
            const enum feat feat = (enum feat)(uintptr_t)list_get(available_feats, available_feat_index);

            const char *text = "[ ] %s";

            if (race_database[selected_race].feats[feat])
            {
                text = "[-] %s (racial)";
            }

            if (class_database[selected_class].feat_progression[feat] == 1)
            {
                text = "[-] %s (class)";
            }

            if (feats[feat])
            {
                text = "[x] %s";
            }

            console_print(
                console,
                1, y++,
                feat == selected_feat ? &color_yellow : &color_white,
                &color_black,
                TCOD_BKGND_NONE,
                TCOD_LEFT,
                text,
                feat_database[feat].name);
        }

        y++;

        console_print(
            console,
            1, y++,
            &color_white,
            &color_black,
            TCOD_BKGND_NONE,
            TCOD_LEFT,
            "Remaining Feats: %d",
            remaining_feats);
    }
    break;
    case STATE_CONFIRM:
    {
        console_print(
            console,
            console_width / 2, 2,
            &color_white,
            &color_black,
            TCOD_BKGND_NONE,
            TCOD_CENTER,
            "Prepare yourself, %s!",
            name);
    }
    break;
    }

    TCOD_console_printn_frame(
        console,
        0, 0, console_width, console_height,
        sizeof(TITLE),
        TITLE,
        &color_white,
        &color_black,
        TCOD_BKGND_NONE,
        false);

    return &create_scene;
}

struct scene create_scene =
    {&init,
     &uninit,
     &handle_event,
     &update};
