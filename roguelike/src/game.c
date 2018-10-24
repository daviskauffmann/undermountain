#include <roguelike/roguelike.h>

struct game *game;

void game_init(void)
{
    game = calloc(1, sizeof(struct game));

    game->state = GAME_STATE_MENU;

    for (int level = 0; level < NUM_MAPS; level++)
    {
        struct map *map = &game->maps[level];

        map_init(map, level);
    }

    game->messages = TCOD_list_new();

    game->player = NULL;

    game->turn = 0;
    game->should_update = true;
    game->should_quit = false;
}

void game_new(void)
{
    game->state = GAME_STATE_PLAY;

    TCOD_sys_delete_file(SAVE_PATH);

    for (int level = 0; level < NUM_MAPS; level++)
    {
        struct map *map = &game->maps[level];

        map_generate(map);
    }

    {
        int level = 0;
        struct map *map = &game->maps[level];
        int x = map->stair_up_x;
        int y = map->stair_up_y;
        struct tile *tile = &map->tiles[x][y];

        game->player = actor_create("Blinky", RACE_HUMAN, CLASS_FIGHTER, FACTION_GOOD, 1, level, x, y);
        game->player->glow = false;

        TCOD_list_push(map->actors, game->player);
        tile->actor = game->player;

        struct item *longsword = item_create(ITEM_TYPE_LONGSWORD, level, x, y);

        TCOD_list_push(map->items, longsword);
        TCOD_list_push(game->player->items, longsword);

        struct item *greatsword = item_create(ITEM_TYPE_GREATSWORD, level, x, y);

        TCOD_list_push(map->items, greatsword);
        TCOD_list_push(game->player->items, greatsword);

        struct item *longbow = item_create(ITEM_TYPE_LONGBOW, level, x, y);

        TCOD_list_push(map->items, longbow);
        TCOD_list_push(game->player->items, longbow);

        struct item *tower_shield = item_create(ITEM_TYPE_TOWER_SHIELD, level, x, y);

        TCOD_list_push(map->items, tower_shield);
        TCOD_list_push(game->player->items, tower_shield);

        struct item *potion_cure_light_wounds = item_create(ITEM_TYPE_POTION_CURE_LIGHT_WOUNDS, level, x, y);

        TCOD_list_push(map->items, potion_cure_light_wounds);
        TCOD_list_push(game->player->items, potion_cure_light_wounds);

        game_log(
            level,
            x,
            y,
            TCOD_white,
            "Hail, %s!",
            game->player->name);
    }
}

void game_save(void)
{
    TCOD_zip_t zip = TCOD_zip_new();

    TCOD_zip_save_to_file(zip, SAVE_PATH);

    TCOD_zip_delete(zip);
}

void game_load(void)
{
    game->state = GAME_STATE_PLAY;

    game_new();
}

void game_update(void)
{
    if (game->state != GAME_STATE_MENU)
    {
        game->state = game->player->dead ? GAME_STATE_LOSE : GAME_STATE_PLAY;

        struct map *map = &game->maps[game->player->level];

        for (void **iterator = TCOD_list_begin(map->actors); iterator != TCOD_list_end(map->actors); iterator++)
        {
            struct actor *actor = *iterator;

            actor_update_flash(actor);
        }

        for (void **iterator = TCOD_list_begin(map->projectiles); iterator != TCOD_list_end(map->projectiles); iterator++)
        {
            struct projectile *projectile = *iterator;

            projectile_update(projectile);

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
}

void game_log(int level, int x, int y, TCOD_color_t color, char *fmt, ...)
{
    if (level != game->player->level ||
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
        if (TCOD_list_size(game->messages) == (console_height / 4) - 2)
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
