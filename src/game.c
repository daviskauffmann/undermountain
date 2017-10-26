#include <stdio.h>
#include <libtcod.h>

#include "game.h"
#include "config.h"
#include "map.h"
#include "actor.h"

TCOD_list_t maps;
int current_map_index;
map_t *current_map;
actor_t *player;

void game_init(void)
{
    maps = TCOD_list_new();
    current_map_index = 0;
    current_map = map_create();
    player = actor_create(current_map, ACTORTYPE_PLAYER, current_map->stair_up_x, current_map->stair_up_y, '@', TCOD_white, 10);
}

void game_load(void)
{
    gamedata_t *gamedata = (gamedata_t *)malloc(sizeof(gamedata_t));

    FILE *file = fopen("save.dat", "r");
    fread(gamedata, sizeof(gamedata_t), 1, file);
    fclose(file);

    TCOD_random_restore(NULL, gamedata->random);
    current_map_index = gamedata->current_map_index;

    maps = TCOD_list_new();
    for (int i = 0; i < gamedata->map_count; i++)
    {
        mapdata_t *mapdata = &gamedata->mapdata[i];

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

            actor->type = actordata->type;
            actor->x = actordata->x;
            actor->y = actordata->y;

            TCOD_list_push(map->actors, actor);

            if (actordata->type == ACTORTYPE_PLAYER)
            {
                player = actor;
            }
        }

        TCOD_list_push(maps, map);
    }

    current_map = TCOD_list_get(maps, current_map_index);
}

void game_save(void)
{
    gamedata_t *gamedata = (gamedata_t *)malloc(sizeof(gamedata_t));

    gamedata->random = TCOD_random_save(NULL);
    gamedata->current_map_index = current_map_index;

    gamedata->mapdata = (mapdata_t *)malloc(sizeof(mapdata_t) * TCOD_list_size(maps));
    gamedata->map_count = 0;
    for (map_t **iterator = (map_t **)TCOD_list_begin(maps);
         iterator != (map_t **)TCOD_list_end(maps);
         iterator++)
    {
        map_t *map = *iterator;

        mapdata_t *mapdata = &gamedata->mapdata[gamedata->map_count];

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

            actordata->type = actor->type;
            actordata->x = actor->x;
            actordata->y = actor->y;

            mapdata->actor_count++;
        }

        gamedata->map_count++;
    }

    FILE *file = fopen("save.dat", "w");
    fwrite(gamedata, sizeof(gamedata_t), 1, file);
    fclose(file);

    free(gamedata);
}

void game_update(void)
{
    for (map_t **iterator = (map_t **)TCOD_list_begin(maps);
         iterator != (map_t **)TCOD_list_end(maps);
         iterator++)
    {
        map_t *map = *iterator;

        map_update(map);
    }
}

static void map_update(map_t *map)
{
    for (actor_t **iterator = (actor_t **)TCOD_list_begin(map->actors);
         iterator != (actor_t **)TCOD_list_end(map->actors);
         iterator++)
    {
        actor_t *actor = *iterator;

        actor_update(map, actor);
    }
}

static void actor_update(map_t *map, actor_t *actor)
{
    if (actor == player)
    {
        return;
    }

    if (TCOD_random_get_int(NULL, 0, 1) == 0)
    {
        TCOD_map_t TCOD_map = map_to_TCOD_map(map);

        map_calc_fov(TCOD_map, actor->x, actor->y, actorinfo[actor->type].sight_radius);

        for (actor_t **iterator = (actor_t **)TCOD_list_begin(map->actors);
             iterator != (actor_t **)TCOD_list_end(map->actors);
             iterator++)
        {
            actor_t *other = *iterator;

            if (other == actor)
            {
                continue;
            }

            if (TCOD_map_is_in_fov(TCOD_map, other->x, other->y))
            {
                // TODO: maybe store the path on the actor somehow so it can be reused
                TCOD_path_t path = map_calc_path(TCOD_map, actor->x, actor->y, other->x, other->y);

                if (TCOD_path_is_empty(path))
                {
                    goto end;
                }

                int x, y;
                if (!TCOD_path_walk(path, &x, &y, false))
                {
                    goto end;
                }

                actor_move(map, actor, x, y);

            end:
                TCOD_path_delete(path);

                break;
            }
        }

        TCOD_map_delete(TCOD_map);
    }
    else
    {
        int dir = TCOD_random_get_int(NULL, 0, 8);
        switch (dir)
        {
        case 0:
            actor_move(map, actor, actor->x, actor->y - 1);
            break;
        case 1:
            actor_move(map, actor, actor->x, actor->y + 1);
            break;
        case 2:
            actor_move(map, actor, actor->x - 1, actor->y);
            break;
        case 3:
            actor_move(map, actor, actor->x + 1, actor->y);
            break;
        }
    }
}

void game_destroy(void)
{
    for (map_t **iterator = (map_t **)TCOD_list_begin(maps);
         iterator != (map_t **)TCOD_list_end(maps);
         iterator++)
    {
        map_t *map = *iterator;

        TCOD_list_clear_and_delete(map->actors);
    }

    TCOD_list_clear_and_delete(maps);
}