#include <stdio.h>
#include <libtcod.h>

#include "save.h"
#include "world.h"

void world_save(world_t *world)
{
    // TODO: this crashes sometimes for unknown reasons
    // may need to bring out the debugger
    worlddata_t *worlddata = (worlddata_t *)malloc(sizeof(worlddata_t));

    worlddata->random = TCOD_random_save(NULL);
    worlddata->current_map_index = world->current_map_index;

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

            actordata->is_player = actor == world->player;
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
    fwrite(worlddata, sizeof(mapdata_t), 1, file);
    fclose(file);

    free(worlddata);
}

world_t *world_load(void)
{
    // TODO: this also crashes for unknown reasons
    // it also always crashes when loading a file from another instance of the game
    // it has only ever been successful when loading a file from the same process
    // could have something to do with pointers?
    worlddata_t *worlddata = (worlddata_t *)malloc(sizeof(worlddata_t));

    FILE *file = fopen("save.dat", "r");
    fread(worlddata, sizeof(worlddata_t), 1, file);
    fclose(file);

    world_t *world = (world_t *)malloc(sizeof(world_t));

    TCOD_random_restore(NULL, worlddata->random);
    world->current_map_index = worlddata->current_map_index;

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
                world->player = actor;
            }
        }

        TCOD_list_push(world->maps, map);
    }

    world->current_map = TCOD_list_get(world->maps, world->current_map_index);

    return world;
}