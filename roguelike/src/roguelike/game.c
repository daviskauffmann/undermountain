#include <roguelike/roguelike.h>

struct game *game;

void game_init(void)
{
    game = calloc(1, sizeof(struct game));

    game->state = GAME_STATE_PLAY;
    game->messages = TCOD_list_new();
    game->player = NULL;
    game->turn = 0;
    game->should_update = true;

    for (int floor = 0; floor < NUM_MAPS; floor++)
    {
        struct map *map = &game->maps[floor];

        map_init(map, floor);
    }
}

void game_new(void)
{
    for (int floor = 0; floor < NUM_MAPS; floor++)
    {
        struct map *map = &game->maps[floor];

        map_generate(map);
    }

    {
        int floor = 0;
        struct map *map = &game->maps[floor];
        int x = map->stair_up_x;
        int y = map->stair_up_y;
        struct tile *tile = &map->tiles[x][y];

        game->player = actor_create("Blinky", RACE_HUMAN, CLASS_FIGHTER, FACTION_GOOD, 1, floor, x, y);

        TCOD_list_push(map->actors, game->player);
        tile->actor = game->player;

        struct item *longsword = item_create(ITEM_TYPE_LONGSWORD, floor, x, y);

        TCOD_list_push(map->items, longsword);
        TCOD_list_push(game->player->items, longsword);

        struct item *greatsword = item_create(ITEM_TYPE_GREATSWORD, floor, x, y);

        TCOD_list_push(map->items, greatsword);
        TCOD_list_push(game->player->items, greatsword);

        struct item *longbow = item_create(ITEM_TYPE_LONGBOW, floor, x, y);

        TCOD_list_push(map->items, longbow);
        TCOD_list_push(game->player->items, longbow);

        struct item *tower_shield = item_create(ITEM_TYPE_TOWER_SHIELD, floor, x, y);

        TCOD_list_push(map->items, tower_shield);
        TCOD_list_push(game->player->items, tower_shield);

        struct item *potion_cure_light_wounds = item_create(ITEM_TYPE_POTION_CURE_LIGHT_WOUNDS, floor, x, y);

        TCOD_list_push(map->items, potion_cure_light_wounds);
        TCOD_list_push(game->player->items, potion_cure_light_wounds);

        game_log(
            floor,
            x,
            y,
            TCOD_white,
            "Hail, %s!",
            game->player->name);
    }
}

void game_save(const char *file)
{
    TCOD_zip_t zip = TCOD_zip_new();

    TCOD_zip_save_to_file(zip, file);

    TCOD_zip_delete(zip);
}

void game_load(const char *file)
{
    // TODO: load state from file
    game_new();
}

void game_update(float delta_time)
{
    game->state = game->player->dead ? GAME_STATE_LOSE : GAME_STATE_PLAY;

    struct map *map = &game->maps[game->player->floor];

    for (void **iterator = TCOD_list_begin(map->actors); iterator != TCOD_list_end(map->actors); iterator++)
    {
        struct actor *actor = *iterator;

        actor_update_flash(actor, delta_time);
    }

    for (void **iterator = TCOD_list_begin(map->projectiles); iterator != TCOD_list_end(map->projectiles); iterator++)
    {
        struct projectile *projectile = *iterator;

        projectile_update(projectile, delta_time);

        if (projectile->destroyed)
        {
            iterator = TCOD_list_remove_iterator(map->projectiles, iterator);

            projectile_destroy(projectile);
        }
    }

    if (game->should_update)
    {
        game->should_update = false;
        game->turn++;

        for (void **iterator = TCOD_list_begin(map->objects); iterator != TCOD_list_end(map->objects); iterator++)
        {
            struct object *object = *iterator;

            if (object->destroyed)
            {
                struct tile *tile = &map->tiles[object->x][object->y];

                tile->object = NULL;
                iterator = TCOD_list_remove_iterator(map->objects, iterator);

                object_destroy(object);

                continue;
            }

            object_calc_light(object);
        }

        for (void **iterator = TCOD_list_begin(map->actors); iterator != TCOD_list_end(map->actors); iterator++)
        {
            struct actor *actor = *iterator;

            actor_calc_light(actor);
        }

        for (void **iterator = TCOD_list_begin(map->actors); iterator != TCOD_list_end(map->actors); iterator++)
        {
            struct actor *actor = *iterator;

            actor_calc_fov(actor);

            if (!actor->dead)
            {
                actor_ai(actor);
            }
        }
    }
}

// TODO: we should proably redo the logging system
// instead of logging messages directly, the game should store all the events that have happened
// the renderer can read the last few events to generate a message
// this allows us to read the entire history of the game and do anything with it
void game_log(int floor, int x, int y, TCOD_color_t color, char *fmt, ...)
{
    if (floor != game->player->floor ||
        !game->player->fov ||
        !TCOD_map_is_in_fov(game->player->fov, x, y))
    {
        return;
    }

    char buffer[256];

    va_list args;
    va_start(args, fmt);
    vsprintf_s(buffer, sizeof(buffer), fmt, args);
    va_end(args);

    char *line_begin = buffer;
    char *line_end;

    do
    {
        if (TCOD_list_size(game->messages) == (50 / 4) - 2) // TODO: hardcoded console_height for now because we're gonna throw all this away soon
        {
            struct message *message = TCOD_list_get(game->messages, 0);

            TCOD_list_remove(game->messages, message);

            message_destroy(message);
        }

        line_end = strchr(line_begin, '\n');

        if (line_end)
        {
            *line_end = '\0';
        }

        struct message *message = message_create(line_begin, color);

        TCOD_list_push(game->messages, message);

        line_begin = line_end + 1;
    }
    while (line_end);
}

void game_quit(void)
{
    for (int i = 0; i < NUM_MAPS; i++)
    {
        struct map *map = &game->maps[i];

        map_reset(map);
    }

    for (void **iterator = TCOD_list_begin(game->messages); iterator != TCOD_list_end(game->messages); iterator++)
    {
        struct message *message = *iterator;

        message_destroy(message);
    }

    TCOD_list_delete(game->messages);

    free(game);
}
