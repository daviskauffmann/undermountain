#include <stdio.h>
#include <SDL.h>
#include <libtcod.h>

#include "CMemLeak.h"
#include "system.h"

/* Game */
typedef enum status_e {
    STATUS_WAITING,
    STATUS_UPDATE,
    STATUS_QUIT
} status_t;

typedef struct game_s
{
    status_t status;
    int turn;
} game_t;

/* Components */
#define MAX_ENTITIES 256
#define ID_UNUSED -1

typedef enum component_e {
    COMPONENT_POSITION,
    COMPONENT_APPEARANCE,
    COMPONENT_INPUT,
    COMPONENT_AI,

    NUM_COMPONENTS
} component_t;

typedef struct position_s
{
    int id;
    int x;
    int y;
} position_t;

typedef struct appearance_s
{
    int id;
    char *name;
    unsigned char glyph;
    TCOD_color_t color;
} appearance_t;

void *components[NUM_COMPONENTS][MAX_ENTITIES];

position_t position_components[MAX_ENTITIES];
appearance_t appearance_components[MAX_ENTITIES];

/* Entities */
typedef struct entity_s
{
    int id;
    void *components[NUM_COMPONENTS];
} entity_t;

entity_t entities[MAX_ENTITIES];

entity_t *entity_create()
{
    for (int i = 0; i < MAX_ENTITIES; i++)
    {
        if (entities[i].id == ID_UNUSED)
        {
            return &entities[i];
        }
    }

    return NULL;
}

void *entity_add_component(entity_t *entity, component_t component)
{
}

void *entity_get_component(entity_t *entity, component_t component)
{
}

/* Systems */
void ECS_init(void)
{
    for (int i = 0; i < MAX_ENTITIES; i++)
    {
        entities[i].id = ID_UNUSED;
        position_components[i].id = ID_UNUSED;
        appearance_components[i].id = ID_UNUSED;
    }
}

void input_system(game_t *game, entity_t *player)
{
    TCOD_key_t key;
    TCOD_mouse_t mouse;
    TCOD_event_t ev = TCOD_sys_check_for_event(TCOD_EVENT_ANY, &key, &mouse);

    switch (ev)
    {
    case TCOD_EVENT_KEY_PRESS:
    {
        switch (key.vk)
        {
        case TCODK_ESCAPE:
        {
            game->status = STATUS_QUIT;

            break;
        }
        }
    }
    }
}

void render_system(void)
{
    for (int i = 0; i < MAX_ENTITIES; i++)
    {
        if (appearance_components[i].id != ID_UNUSED)
        {
        }
    }
}

int main(int argc, char *argv[])
{
    system_init();

    game_t game = {
        .status = STATUS_WAITING,
        .turn = 0};

    entity_t *player = entity_create();

    while (!TCOD_console_is_window_closed())
    {
        input_system(&game, player);

        if (game.status == STATUS_QUIT)
        {
            break;
        }
    }

    SDL_Quit();

    return 0;
}