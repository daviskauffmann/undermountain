#include "create_scene.h"

#include "../../config.h"
#include "../../game/actor.h"
#include "../../game/assets.h"
#include "../../game/color.h"
#include "../../game/world.h"
#include "../../print.h"
#include "../../scene.h"
#include "../game/game_scene.h"
#include "../menu/menu_scene.h"
#include <libtcod.h>

enum state
{
    STATE_STORY,
    STATE_NAME,
    STATE_RACE,
    STATE_CLASS,
    STATE_ABILITY_SCORES,
    STATE_CONFIRM,

    NUM_STATES
};

static const int recommended_ability_scores[NUM_ABILITIES][NUM_CLASSES] = {
    [CLASS_FIGHTER] = {
        [ABILITY_STRENGTH] = 18,
        [ABILITY_DEXTERITY] = 18,
        [ABILITY_CONSTITUTION] = 18,
        [ABILITY_INTELLIGENCE] = 18,
    },
    [CLASS_ROGUE] = {
        [ABILITY_STRENGTH] = 18,
        [ABILITY_DEXTERITY] = 18,
        [ABILITY_CONSTITUTION] = 18,
        [ABILITY_INTELLIGENCE] = 18,
    },
    [CLASS_WIZARD] = {
        [ABILITY_STRENGTH] = 18,
        [ABILITY_DEXTERITY] = 18,
        [ABILITY_CONSTITUTION] = 18,
        [ABILITY_INTELLIGENCE] = 18,
    },
};

static enum state state;
static bool default_name_modified;

static char name[32];
static enum race race;
static enum class class;
static int ability_points;
static int ability_scores[NUM_ABILITIES];

static int calc_ability_score_cost(const enum ability ability)
{
    const int ability_score = ability_scores[ability];

    if (ability_score < 15)
    {
        return 1;
    }
    else if (ability_score < 17)
    {
        return 2;
    }
    else if (ability_score < 18)
    {
        return 3;
    }

    return 0;
}

static void init(const struct scene *const previous_scene)
{
    previous_scene;

    state = STATE_STORY;
    default_name_modified = false;

    strcpy(name, "Adventurer");
    race = RACE_HUMAN;
    class = CLASS_FIGHTER;
    ability_points = 0;
    for (enum ability ability = 0; ability < NUM_ABILITIES; ability++)
    {
        ability_scores[ability] = recommended_ability_scores[class][ability];
    }
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
                    race,
                    class,
                    ability_scores,
                    FACTION_ADVENTURER,
                    0,
                    0,
                    0);

                hero->equipment[EQUIP_SLOT_WEAPON] = item_new(ITEM_TYPE_LONGSWORD, 0, 0, 0, 1);
                list_add(hero->known_spell_types, (void *)SPELL_TYPE_MINOR_HEAL);
                list_add(hero->known_spell_types, (void *)SPELL_TYPE_FIREBALL);
                hero->readied_spell_type = SPELL_TYPE_FIREBALL;

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

            if (state == 0)
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
            case STATE_RACE:
            {
                if (alpha >= PLAYER_RACE_BEGIN && alpha <= PLAYER_RACE_END)
                {
                    race = alpha;
                }
            }
            break;
            case STATE_CLASS:
            {
                if (alpha >= PLAYER_CLASS_BEGIN && alpha <= PLAYER_CLASS_END)
                {
                    class = alpha;

                    for (enum ability ability = 0; ability < NUM_ABILITIES; ability++)
                    {
                        ability_scores[ability] = recommended_ability_scores[class][ability];
                    }
                }
            }
            break;
            case STATE_ABILITY_SCORES:
            {
                if (alpha < NUM_ABILITIES)
                {
                    const int cost = calc_ability_score_cost(alpha);

                    if (ability_points >= cost && ability_scores[alpha] < 18)
                    {
                        ability_scores[alpha]++;
                        ability_points -= cost;
                    }
                }
                else if (event->key.keysym.sym == SDLK_r)
                {
                    if (event->key.keysym.mod & KMOD_SHIFT)
                    {
                        for (enum ability ability = 0; ability < NUM_ABILITIES; ability++)
                        {
                            ability_scores[ability] = 8;
                        }
                        ability_points = NUM_ABILITIES * 5;
                    }
                    else
                    {
                        for (enum ability ability = 0; ability < NUM_ABILITIES; ability++)
                        {
                            ability_scores[ability] = recommended_ability_scores[class][ability];
                        }
                        ability_points = 0;
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
        console_print(
            console,
            1,
            y++,
            &color_white,
            &color_black,
            TCOD_BKGND_NONE,
            TCOD_LEFT,
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
                &color_white,
                &color_black,
                TCOD_BKGND_NONE,
                TCOD_LEFT,
                "%c) %s",
                race + SDLK_a,
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
            "Selected: %s",
            race_database[race].name);

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
            size_database[race_database[race].size].name);

        console_print(
            console,
            1,
            y++,
            &color_white,
            &color_black,
            TCOD_BKGND_NONE,
            TCOD_LEFT,
            "Speed: %.1f",
            race_database[race].speed);

        y++;

        // TODO: actually implement these features
        switch (race)
        {
        case RACE_HUMAN:
        {
            console_print(
                console,
                1,
                y++,
                &color_white,
                &color_black,
                TCOD_BKGND_NONE,
                TCOD_LEFT,
                "Features: None");
        }
        break;
        case RACE_ELF:
        {
            console_print(
                console,
                1,
                y++,
                &color_white,
                &color_black,
                TCOD_BKGND_NONE,
                TCOD_LEFT,
                "Features: +2 Dexterity, -2 Constitution, Darkvision");
        }
        break;
        case RACE_DWARF:
        {
            console_print(
                console,
                1,
                y++,
                &color_white,
                &color_black,
                TCOD_BKGND_NONE,
                TCOD_LEFT,
                "Features: +2 Constitution, -2 Intelligence");
        }
        break;
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
                &color_white,
                &color_black,
                TCOD_BKGND_NONE,
                TCOD_LEFT,
                "%c) %s",
                class + SDLK_a,
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
            "Selected: %s",
            class_database[class].name);

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
            class_database[class].hit_die);

        console_print(
            console,
            1,
            y++,
            &color_white,
            &color_black,
            TCOD_BKGND_NONE,
            TCOD_LEFT,
            "Mana die: %s",
            class_database[class].mana_die);

        console_print(
            console,
            1,
            y++,
            &color_white,
            &color_black,
            TCOD_BKGND_NONE,
            TCOD_LEFT,
            "Base attack bonus: %s (x%.2f)",
            base_attack_bonus_progression_database[class_database[class].base_attack_bonus_progression].name,
            base_attack_bonus_progression_database[class_database[class].base_attack_bonus_progression].multiplier);
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
            const int cost = calc_ability_score_cost(ability);

            console_print(
                console,
                1,
                y++,
                &color_white,
                &color_black,
                TCOD_BKGND_NONE,
                TCOD_LEFT,
                "%c) %s: %d (%d)",
                ability + SDLK_a,
                ability_database[ability].name,
                ability_scores[ability],
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
            "r) Recommended");
        console_print(
            console,
            1,
            y++,
            &color_white,
            &color_black,
            TCOD_BKGND_NONE,
            TCOD_LEFT,
            "R) Reset");

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
