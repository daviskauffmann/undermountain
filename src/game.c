#include <stdio.h>
#include <stdint.h>
#include <libtcod.h>

#include "game.h"
#include "world.h"
#include "map.h"
#include "actor.h"

world_t *world;
uint8_t current_map_index;
map_t *current_map;
actor_t *player;

void game_init(void)
{
    world = world_create();
    current_map_index = 0;
    current_map = map_create();
    player = actor_create(current_map, current_map->stair_up_x, current_map->stair_up_y, '@', TCOD_white, 10);
}

void game_load(void)
{
    // TODO: this crashes for unknown reasons
    // it also always crashes when loading a file from another instance of the game
    // it has only ever been successful when loading a file from the same process
    // could have something to do with pointers?
    gamedata_t *gamedata = (gamedata_t *)malloc(sizeof(gamedata_t));

    FILE *file = fopen("save.dat", "r");
    fread(gamedata, sizeof(gamedata_t), 1, file);
    fclose(file);

    TCOD_random_restore(NULL, gamedata->random);
    current_map_index = gamedata->current_map_index;

    worlddata_t *worlddata = gamedata->worlddata;
    world = (world_t *)malloc(sizeof(world_t));

    world->maps = TCOD_list_new();
    for (int i = 0; i < worlddata->map_count; i++)
    {
        mapdata_t *mapdata = &worlddata->mapdata[i];
        map_t *map = (map_t *)malloc(sizeof(map_t));

        map->stair_down_x = mapdata->stair_down_x;
        map->stair_down_y = mapdata->stair_down_y;
        map->stair_up_x = mapdata->stair_up_x;
        map->stair_up_y = mapdata->stair_up_y;

        for (int x = 0; x < MAP_WIDTH; x++)
        {
            for (int y = 0; y < MAP_HEIGHT; y++)
            {
                tiledata_t *tiledata = &mapdata->tiledata[x][y];
                tile_t *tile = &map->tiles[x][y];

                tile->type = tiledata->type;
                tile->seen = tiledata->seen;
            }
        }

        map->rooms = TCOD_list_new();
        for (int j = 0; j < mapdata->room_count; j++)
        {
            roomdata_t *roomdata = &mapdata->roomdata[j];
            room_t *room = (room_t *)malloc(sizeof(room_t));

            room->x = roomdata->x;
            room->y = roomdata->y;
            room->w = roomdata->w;
            room->h = roomdata->h;

            TCOD_list_push(map->rooms, room);
        }

        map->actors = TCOD_list_new();
        for (int j = 0; j < mapdata->actor_count; j++)
        {
            actordata_t *actordata = &mapdata->actordata[j];
            actor_t *actor = (actor_t *)malloc(sizeof(actor_t));

            actor->x = actordata->x;
            actor->y = actordata->y;
            actor->glyph = actordata->glyph;
            actor->color = actordata->color;
            actor->sight_radius = actordata->sight_radius;

            TCOD_list_push(map->actors, actor);

            if (actordata->is_player)
            {
                player = actor;
            }
        }

        TCOD_list_push(world->maps, map);
    }

    current_map = TCOD_list_get(world->maps, current_map_index);
}

void game_save(void)
{
    // TODO: this crashes for unknown reasons
    // may need to bring out the debugger
    gamedata_t *gamedata = (gamedata_t *)malloc(sizeof(gamedata_t));

    gamedata->random = TCOD_random_save(NULL);
    gamedata->current_map_index = current_map_index;
    gamedata->worlddata = (worlddata_t *)malloc(sizeof(worlddata_t));

    worlddata_t *worlddata = gamedata->worlddata;

    worlddata->mapdata = (mapdata_t *)malloc(sizeof(mapdata_t) * TCOD_list_size(world->maps));
    worlddata->map_count = 0;
    for (map_t **iterator = (map_t **)TCOD_list_begin(world->maps);
         iterator != (map_t **)TCOD_list_end(world->maps);
         iterator++)
    {
        map_t *map = *iterator;
        mapdata_t *mapdata = &worlddata->mapdata[worlddata->map_count];

        mapdata->stair_down_x = map->stair_down_x;
        mapdata->stair_down_y = map->stair_down_y;
        mapdata->stair_up_x = map->stair_up_x;
        mapdata->stair_up_y = map->stair_up_y;

        for (int x = 0; x < MAP_WIDTH; x++)
        {
            for (int y = 0; y < MAP_HEIGHT; y++)
            {
                tile_t *tile = &map->tiles[x][y];
                tiledata_t *tiledata = &mapdata->tiledata[x][y];

                tiledata->type = tile->type;
                tiledata->seen = tile->seen;
            }
        }

        mapdata->roomdata = (roomdata_t *)malloc(sizeof(roomdata_t) * TCOD_list_size(map->rooms));
        mapdata->room_count = 0;
        for (room_t **iterator = (room_t **)TCOD_list_begin(map->rooms);
             iterator != (room_t **)TCOD_list_end(map->rooms);
             iterator++)
        {
            room_t *room = *iterator;
            roomdata_t *roomdata = &mapdata->roomdata[mapdata->room_count];

            roomdata->x = room->x;
            roomdata->y = room->y;
            roomdata->w = room->w;
            roomdata->h = room->h;

            mapdata->room_count++;
        }

        mapdata->actordata = (actordata_t *)malloc(sizeof(actordata_t) * TCOD_list_size(map->actors));
        mapdata->actor_count = 0;
        for (actor_t **iterator = (actor_t **)TCOD_list_begin(map->actors);
             iterator != (actor_t **)TCOD_list_end(map->actors);
             iterator++)
        {
            actor_t *actor = *iterator;
            actordata_t *actordata = &mapdata->actordata[mapdata->actor_count];

            actordata->is_player = actor == player;
            actordata->x = actor->x;
            actordata->y = actor->y;
            actordata->glyph = actor->glyph;
            actordata->color = actor->color;
            actordata->sight_radius = actor->sight_radius;

            mapdata->actor_count++;
        }

        worlddata->map_count++;
    }

    FILE *file = fopen("save.dat", "w");
    fwrite(gamedata, sizeof(gamedata_t), 1, file);
    fclose(file);

    free(gamedata);
}