#include <roguelike/roguelike.h>

struct game *game;

// NOTE: there is a memory leak when freeing the game
// something is not being cleaned up properly because less memory is freed when calling game_quit() than was allocated with game_init()
// not sure what is causing this, so keep looking into it
// UPDATE: it seems to be coming from namegen

// TODO: optimize
// decide on a target ms per turn, maybe 16ms
// the game_turn() function should never take longer than that to run

// TODO: traps

// TOOD: chests

// TOOD: redo map generation
// no need for overworld map, game will start on the first dungeon
// we need to define a win condition for the game, probably just grabbing an amulet or something and returning to the entrance?
// ascending the first floor stairs will end the game
// maps should have multiple stairs
// the stairs should geographically connect to the above/below floors, so that will have to be taken into account during map generation
// thus, the stair object should be a little more sophisticated. it should hold a stair_id which connects it to another stair with the same id
// using the stair will just teleport the player to the other stair
// this simplifies the code a little bit because we no longer need to have ascend() and descend() functions, just one that handles moving from one stair to the other
// making a system that is aware of the layout of the entire dungeon allows for the possibility of "pit traps", which make the player (or creatures) fall to a lower floor
// these pit traps will be generated in a similar way to the stairs, in that they can only appear if the same tile coordinate on the map below is a floor
// in addition to all that, there should be ladders that connect to smaller maps that contain loot and enemies
// these maps have no way out other than the entrance and are smaller

// TODO: implement processing of maps the player is not on

// TODO: not storing all maps in memory?
// if we have a small number of maps, this might not be a problem
// might interfere with the above todo

// TODO: have different TCOD_random_t instances for different things

// TODO: possibly redo input to be more like commit 67aa306?
// look at game.c, line 180 in the game_update() function
// this type of input allows for multiple player characters or playable character switching on the fly
// a practical gameplay application might be a mind control spell
// unfortunately, with the implementation i did before, it resulted in each actor taking an entire frame to process
// not because of computation time, but because the actor would do their ai that frame, and then on the next update, it would hit the next actor to process them, and so on
// some research and testing needs to be done to see if this is just a necessary consequence or maybe there is a smarter way to accomplish this
// here is my proposal:
// give all actors a bool to say whether they are input controlled or not
// game_update()
//   if waiting_for_input is true
//     return
//   loop through all the maps
//     loop through all the actors
//       if the current actor has enough energy
//         remove some energy
//         if the current actor is input controlled
//           set waiting_for_input to true
//           set the game->player to the current actor (so we need to independently track the main character for win/lose state reasons?)
//           break out of the loop
//         otherwise
//           process ai for this actor
// state_game->input()
//   took_turn can be local now, set to false at the beginning
//   do everything as normal, applying input to the current game->player and storing the result in took_turn
//   if took_turn is true and game->player->energy is over a threshold
//     instead of calling game_turn(), just set waiting_for_input to false

// TOOD: communication with platform layer?
// for stuff like logging and file system
// perhaps game_init() should accept a struct containing some function pointers to platform layer stuff
// or just a header that the game provides with function declarations and the platform must define them

void game_init(void)
{
    game = malloc(sizeof(struct game));

    if (!game)
    {
        printf("Couldn't allocate game\n");

        return;
    }

    game->state = GAME_STATE_PLAY;
    game->messages = TCOD_list_new();
    game->player = NULL;
    game->turn = 0;

    for (int floor = 0; floor < NUM_MAPS; floor++)
    {
        struct map *map = &game->maps[floor];

        map_init(map, floor);
    }

    assets_load();
}

// TODO: this should accept an actor which will become the player, presumably passed from a character creation menu
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

        {
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

        {
            int x = map->stair_up_x + 1;
            int y = map->stair_up_y + 1;
            struct tile *tile = &map->tiles[x][y];

            struct actor *pet = actor_create("Spot", RACE_ANIMAL, CLASS_ANIMAL, FACTION_GOOD, 1, floor, x, y);
            pet->leader = game->player;
            pet->speed = 1.0f;

            TCOD_list_push(map->actors, pet);
            tile->actor = pet;
        }
    }
}

void game_save(const char *filename)
{
    TCOD_zip_t zip = TCOD_zip_new();

    TCOD_zip_save_to_file(zip, filename);

    // TODO: save game to zip

    TCOD_zip_delete(zip);

    printf("Game saved\n");
}

void game_load(const char *filename)
{

    TCOD_zip_t zip = TCOD_zip_new();

    TCOD_zip_load_from_file(zip, filename);

    // TODO: load game from zip

    TCOD_zip_delete(zip);

    // DEBUG: just start a new game
    game_new();

    printf("Game loaded\n");
}

void game_update(void)
{
    game->state = game->player->dead ? GAME_STATE_LOSE : GAME_STATE_PLAY;

    struct map *map = &game->maps[game->player->floor];

    TCOD_LIST_FOREACH(map->actors)
    {
        struct actor *actor = *iterator;

        actor_update_flash(actor);
    }

    TCOD_LIST_FOREACH(map->projectiles)
    {
        struct projectile *projectile = *iterator;

        projectile_update(projectile);

        if (projectile->destroyed)
        {
            iterator = TCOD_list_remove_iterator(map->projectiles, iterator);

            projectile_destroy(projectile);
        }
    }
}

void game_turn(void)
{
    game->turn++;

    struct map *map = &game->maps[game->player->floor];

    TCOD_LIST_FOREACH(map->objects)
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

    TCOD_LIST_FOREACH(map->actors)
    {
        struct actor *actor = *iterator;

        actor_calc_light(actor);
    }

    TCOD_LIST_FOREACH(map->actors)
    {
        struct actor *actor = *iterator;

        actor_calc_fov(actor);
        actor_ai(actor);
    }
}

// TODO: we should proably redo the logging system
// instead of logging messages directly, the game should store all the events that have happened
// the renderer can read the last few events to generate a message (using assets as template strings)
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

    TCOD_LIST_FOREACH(game->messages)
    {
        struct message *message = *iterator;

        message_destroy(message);
    }

    TCOD_list_delete(game->messages);

    free(game);

    game = NULL;

    assets_unload();
}
