#include <stdio.h>
#include <libtcod.h>

#include "game.h"
#include "config.h"
#include "world.h"

void game_init(void)
{
    current_map_index = 0;
    current_map = map_create();
    player = actor_create(current_map, ACTORTYPE_PLAYER, current_map->stair_up_x, current_map->stair_up_y);
    torch = false;
}

void game_update(void)
{
    view_right = screen_width;
    view_bottom = screen_height;
    view_left = player->x - view_right / 2;
    view_top = player->y - view_bottom / 2;

    if (CONSTRAIN_VIEW)
    {
        view_left = view_left < 0
                        ? 0
                        : view_left + view_right > MAP_WIDTH
                              ? MAP_WIDTH - view_right
                              : view_left;
        view_top = view_top < 0
                       ? 0
                       : view_top + view_bottom > MAP_HEIGHT
                             ? MAP_HEIGHT - view_bottom
                             : view_top;
    }
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
            actor->target_x = -1;
            actor->target_y = -1;

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