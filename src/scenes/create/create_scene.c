#include "create_scene.h"

#include "../../config.h"
#include "../../game/actor.h"
#include "../../game/color.h"
#include "../../game/data.h"
#include "../../game/world.h"
#include "../../print.h"
#include "../../scene.h"
#include "../game/game_scene.h"
#include "../menu/menu_scene.h"
#include "state.h"
#include <libtcod.h>

static enum state state;

static char name[32];
static bool default_name_modified;

static enum race selected_race;

static enum class selected_class;

static int ability_points;
static int ability_scores[NUM_ABILITIES];
static enum ability selected_ability;

static bool feats[NUM_FEATS];

static int calc_ability_score_cost(const int ability_score)
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
    for (enum ability ability = 0; ability < NUM_ABILITIES; ability++)
    {
        ability_scores[ability] = 8;
    }

    ability_points = NUM_ABILITIES * 5;
}

static void set_recommended_ability_scores(void)
{
    for (enum ability ability = 0; ability < NUM_ABILITIES; ability++)
    {
        ability_scores[ability] = class_database[selected_class].default_ability_scores[ability];
    }

    ability_points = 0;
}

static void init(const struct scene *const previous_scene)
{
    previous_scene;

    state = STATE_STORY;

    strcpy(name, "Adventurer");
    default_name_modified = false;

    selected_race = RACE_HUMAN;

    selected_class = CLASS_FIGHTER;

    selected_ability = ABILITY_STRENGTH;
    set_recommended_ability_scores();
}

static void uninit(void)
{
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
                struct actor *hero = actor_new(
                    name,
                    selected_race,
                    selected_class,
                    1,
                    ability_scores,
                    feats,
                    FACTION_ADVENTURER,
                    0,
                    0,
                    0);

                for (enum equip_slot equip_slot = EQUIP_SLOT_NONE + 1; equip_slot < NUM_EQUIP_SLOTS; equip_slot++)
                {
                    const enum item_type item_type = class_database[selected_class].starting_equipment[equip_slot];

                    if (item_type != ITEM_TYPE_NONE)
                    {
                        hero->equipment[equip_slot] = item_new(item_type, 0, 0, 0, 1);
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

                hero->light_type = LIGHT_TYPE_TORCH;

                world_init();
                world_create(hero);

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
                    set_recommended_ability_scores();
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

                set_recommended_ability_scores();
            }
            break;
            case STATE_ABILITY_SCORES:
            {
                if (selected_ability == NUM_ABILITIES - 1)
                {
                    selected_ability = 0;
                }
                else
                {
                    selected_ability++;
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
                const int cost = calc_ability_score_cost(ability_scores[selected_ability]);

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
                const int cost = calc_ability_score_cost(ability_scores[selected_ability] + 1);

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

                set_recommended_ability_scores();
            }
            break;
            case STATE_ABILITY_SCORES:
            {
                if (selected_ability == 0)
                {
                    selected_ability = NUM_ABILITIES - 1;
                }
                else
                {
                    selected_ability--;
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
        1,
        console_height - 2,
        &color_white,
        &color_black,
        TCOD_BKGND_NONE,
        TCOD_LEFT,
        "<- ESC");

    console_print(
        console,
        console_width - 2,
        console_height - 2,
        &color_white,
        &color_black,
        TCOD_BKGND_NONE,
        TCOD_RIGHT,
        "ENTER ->");

    int y = 1;

    switch (state)
    {
    case STATE_STORY:
    {
        y++;

        console_print(
            console,
            console_width / 2,
            y++,
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
            1,
            y++,
            &color_white,
            &color_black,
            TCOD_BKGND_NONE,
            TCOD_LEFT,
            "What is your name?");

        TCOD_console_printn_frame(
            console,
            1, y++, 20, 3,
            0,
            NULL,
            &color_white,
            &color_black,
            TCOD_BKGND_NONE,
            false);
        TCOD_console_printf(
            console,
            2, y,
            "%s",
            name);
    }
    break;
    case STATE_RACE:
    {
        console_print(
            console,
            1,
            y++,
            &color_white,
            &color_black,
            TCOD_BKGND_NONE,
            TCOD_LEFT,
            "What is your race?");

        for (enum race race = PLAYER_RACE_BEGIN; race <= PLAYER_RACE_END; race++)
        {
            console_print(
                console,
                1,
                y++,
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
            1,
            y++,
            &color_white,
            &color_black,
            TCOD_BKGND_NONE,
            TCOD_LEFT,
            "Size: %s",
            size_database[race_database[selected_race].size].name);

        console_print(
            console,
            1,
            y++,
            &color_white,
            &color_black,
            TCOD_BKGND_NONE,
            TCOD_LEFT,
            "Speed: %.1f",
            race_database[selected_race].speed);

        console_print(
            console,
            1,
            y++,
            &color_white,
            &color_black,
            TCOD_BKGND_NONE,
            TCOD_LEFT,
            "Feats:");
        for (enum feat feat = 0; feat < NUM_FEATS; feat++)
        {
            if (race_database[selected_race].feats[feat])
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
    }
    break;
    case STATE_CLASS:
    {
        console_print(
            console,
            1,
            y++,
            &color_white,
            &color_black,
            TCOD_BKGND_NONE,
            TCOD_LEFT,
            "What is your class?");

        for (enum class class = PLAYER_CLASS_BEGIN; class <= PLAYER_CLASS_END; class ++)
        {
            console_print(
                console,
                1,
                y++,
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
            1,
            y++,
            &color_white,
            &color_black,
            TCOD_BKGND_NONE,
            TCOD_LEFT,
            "Hit die: %s",
            class_database[selected_class].hit_die);

        console_print(
            console,
            1,
            y++,
            &color_white,
            &color_black,
            TCOD_BKGND_NONE,
            TCOD_LEFT,
            "Mana die: %s",
            class_database[selected_class].mana_die);

        console_print(
            console,
            1,
            y++,
            &color_white,
            &color_black,
            TCOD_BKGND_NONE,
            TCOD_LEFT,
            "Base attack bonus: %s (level x%.2f)",
            base_attack_bonus_progression_database[class_database[selected_class].base_attack_bonus_progression].name,
            base_attack_bonus_progression_database[class_database[selected_class].base_attack_bonus_progression].multiplier);

        console_print(
            console,
            1,
            y++,
            &color_white,
            &color_black,
            TCOD_BKGND_NONE,
            TCOD_LEFT,
            "Feats:");
        for (enum feat feat = 0; feat < NUM_FEATS; feat++)
        {
            if (class_database[selected_class].feats[feat])
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
    }
    break;
    case STATE_ABILITY_SCORES:
    {
        console_print(
            console,
            1,
            y++,
            &color_white,
            &color_black,
            TCOD_BKGND_NONE,
            TCOD_LEFT,
            "What are your abilities?");

        for (enum ability ability = 0; ability < NUM_ABILITIES; ability++)
        {
            const int modifier = (ability_scores[ability] - 10) / 2;
            const int cost = calc_ability_score_cost(ability_scores[ability] + 1);

            console_print(
                console,
                1,
                y++,
                ability == selected_ability ? &color_yellow : &color_white,
                &color_black,
                TCOD_BKGND_NONE,
                TCOD_LEFT,
                "%s: %d (%d) (cost: %d)",
                ability_database[ability].name,
                ability_scores[ability],
                modifier,
                cost);
        }

        y++;

        console_print(
            console,
            1,
            y++,
            &color_white,
            &color_black,
            TCOD_BKGND_NONE,
            TCOD_LEFT,
            "Remaining Points: %d",
            ability_points);

        y++;

        console_print(
            console,
            1,
            y++,
            &color_white,
            &color_black,
            TCOD_BKGND_NONE,
            TCOD_LEFT,
            "%s",
            ability_database[selected_ability].description);

        console_print(
            console,
            1,
            console_height - 4,
            &color_white,
            &color_black,
            TCOD_BKGND_NONE,
            TCOD_LEFT,
            "right) Increase, left) Decrease, r) Recommended");
    }
    break;
    case STATE_CONFIRM:
    {
        console_print(
            console,
            1,
            y++,
            &color_white,
            &color_black,
            TCOD_BKGND_NONE,
            TCOD_LEFT,
            "Prepare yourself, %s!",
            name);
    }
    break;
    }

    y++;

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
