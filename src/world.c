#include <stdio.h>
#include <libtcod.h>

#include "world.h"
#include "map.h"

tileinfo_t tileinfo[NB_TILETYPES];
TCOD_list_t maps;
uint8_t current_map_index;
map_t *current_map;
actor_t *player;

void world_init(void)
{
    maps = TCOD_list_new();
    current_map_index = 0;
    current_map = map_create();
    player = actor_create(current_map, MAP_WIDTH / 2, MAP_HEIGHT / 2, '@', TCOD_white, 10);

    map_draw(current_map);
}

void world_save(void)
{
    FILE *file = fopen("save.dat", "w");

    worlddata_t *worlddata = (worlddata_t *)malloc(sizeof(worlddata_t));

    worlddata->random = TCOD_random_save(NULL);
    worlddata->current_map_index = current_map_index;

    int i = 0;
    for (map_t **iterator = (map_t **)TCOD_list_begin(maps);
         iterator != (map_t **)TCOD_list_end(maps);
         iterator++)
    {
        map_t *map = *iterator;
        mapdata_t *mapdata = &worlddata->mapdata[i];

        mapdata->valid = true;
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

        int j = 0;
        for (actor_t **iterator = (actor_t **)TCOD_list_begin(map->actors);
             iterator != (actor_t **)TCOD_list_end(map->actors);
             iterator++)
        {
            actor_t *actor = *iterator;
            actordata_t *actordata = &mapdata->actordata[j];

            actordata->valid = true;
            actordata->is_player = actor == player;
            actordata->x = actor->x;
            actordata->y = actor->y;
            actordata->glyph = actor->glyph;
            actordata->color = actor->color;
            actordata->sight_radius = actor->sight_radius;

            j++;
        }

        i++;
    }

    fwrite(worlddata, sizeof(mapdata_t), 1, file);

    fclose(file);
}

void world_load(void)
{
    world_destroy();

    maps = TCOD_list_new();

    FILE *file = fopen("save.dat", "r");

    worlddata_t *worlddata = (worlddata_t *)malloc(sizeof(worlddata_t));

    fread(worlddata, sizeof(worlddata_t), 1, file);

    TCOD_random_restore(NULL, worlddata->random);
    current_map_index = worlddata->current_map_index;

    for (int i = 0; i < 255; i++)
    {
        mapdata_t *mapdata = &worlddata->mapdata[i];

        if (!mapdata->valid)
        {
            continue;
        }

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

        for (int j = 0; j < 255; j++)
        {
            actordata_t *actordata = &mapdata->actordata[j];

            if (!actordata->valid)
            {
                continue;
            }

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

        TCOD_list_push(maps, map);
    }

    current_map = TCOD_list_get(maps, current_map_index);

    fclose(file);

    map_draw(current_map);
}

void world_destroy(void)
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