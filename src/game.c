#include <libtcod/libtcod.h>
#include <malloc.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include "config.h"
#include "actor.h"
#include "game.h"
#include "input.h"
#include "map.h"
#include "message.h"
#include "projectile.h"

struct game *game_create(void)
{
    struct game *game = malloc(sizeof(struct game));

    game->tile_common.shadow_color = TCOD_color_RGB(16, 16, 32);

    game->tile_info[TILE_EMPTY].name = "Empty";
    game->tile_info[TILE_EMPTY].glyph = ' ';
    game->tile_info[TILE_EMPTY].color = TCOD_white;
    game->tile_info[TILE_EMPTY].is_transparent = true;
    game->tile_info[TILE_EMPTY].is_walkable = true;

    game->tile_info[TILE_FLOOR].name = "Floor";
    game->tile_info[TILE_FLOOR].glyph = '.';
    game->tile_info[TILE_FLOOR].color = TCOD_white;
    game->tile_info[TILE_FLOOR].is_transparent = true;
    game->tile_info[TILE_FLOOR].is_walkable = true;

    game->tile_info[TILE_WALL].name = "Wall";
    game->tile_info[TILE_WALL].glyph = '#';
    game->tile_info[TILE_WALL].color = TCOD_white;
    game->tile_info[TILE_WALL].is_transparent = false;
    game->tile_info[TILE_WALL].is_walkable = false;

    game->object_common.__placeholder = 0;

    game->object_info[OBJECT_DOOR_CLOSED].name = "Closed Door";
    game->object_info[OBJECT_DOOR_CLOSED].glyph = '+';
    game->object_info[OBJECT_DOOR_CLOSED].is_transparent = false;
    game->object_info[OBJECT_DOOR_CLOSED].is_walkable = false;

    game->object_info[OBJECT_DOOR_OPEN].name = "Open Door";
    game->object_info[OBJECT_DOOR_OPEN].glyph = '-';
    game->object_info[OBJECT_DOOR_OPEN].is_transparent = true;
    game->object_info[OBJECT_DOOR_OPEN].is_walkable = true;

    game->object_info[OBJECT_STAIR_DOWN].name = "Stair Down";
    game->object_info[OBJECT_STAIR_DOWN].glyph = '>';
    game->object_info[OBJECT_STAIR_DOWN].is_transparent = true;
    game->object_info[OBJECT_STAIR_DOWN].is_walkable = true;

    game->object_info[OBJECT_STAIR_UP].name = "Stair Up";
    game->object_info[OBJECT_STAIR_UP].glyph = '<';
    game->object_info[OBJECT_STAIR_UP].is_transparent = true;
    game->object_info[OBJECT_STAIR_UP].is_walkable = true;

    game->object_info[OBJECT_ALTAR].name = "Altar";
    game->object_info[OBJECT_ALTAR].glyph = '_';
    game->object_info[OBJECT_ALTAR].is_transparent = true;
    game->object_info[OBJECT_ALTAR].is_walkable = false;

    game->object_info[OBJECT_FOUNTAIN].name = "Fountain";
    game->object_info[OBJECT_FOUNTAIN].glyph = '{';
    game->object_info[OBJECT_FOUNTAIN].is_transparent = true;
    game->object_info[OBJECT_FOUNTAIN].is_walkable = false;

    game->object_info[OBJECT_THRONE].name = "Throne";
    game->object_info[OBJECT_THRONE].glyph = '\\';
    game->object_info[OBJECT_THRONE].is_transparent = true;
    game->object_info[OBJECT_THRONE].is_walkable = false;

    game->object_info[OBJECT_TORCH].name = "Torch";
    game->object_info[OBJECT_TORCH].glyph = '*';
    game->object_info[OBJECT_TORCH].is_transparent = true;
    game->object_info[OBJECT_TORCH].is_walkable = false;

    game->actor_common.glow_radius = 5;
    game->actor_common.glow_color = TCOD_white;
    game->actor_common.torch_radius = 10;
    game->actor_common.torch_color = TCOD_light_amber;

    game->race_info[RACE_HUMAN].name = "Human";
    game->race_info[RACE_HUMAN].glyph = '@';
    game->race_info[RACE_HUMAN].energy_per_turn = 0.5f;

    game->race_info[RACE_ELF].name = "Elf";
    game->race_info[RACE_ELF].glyph = '@';
    game->race_info[RACE_ELF].energy_per_turn = 0.6f;

    game->race_info[RACE_DWARF].name = "Dwarf";
    game->race_info[RACE_DWARF].glyph = '@';
    game->race_info[RACE_DWARF].energy_per_turn = 0.4f;

    game->race_info[RACE_ORC].name = "Orc";
    game->race_info[RACE_ORC].glyph = 'o';
    game->race_info[RACE_ORC].energy_per_turn = 0.5f;

    game->race_info[RACE_BUGBEAR].name = "Bugbear";
    game->race_info[RACE_BUGBEAR].glyph = 'b';
    game->race_info[RACE_BUGBEAR].energy_per_turn = 0.5f;

    game->race_info[RACE_ZOMBIE].name = "Zombie";
    game->race_info[RACE_ZOMBIE].glyph = 'z';
    game->race_info[RACE_ZOMBIE].energy_per_turn = 0.3f;

    game->class_info[CLASS_FIGHTER].name = "Fighter";
    game->class_info[CLASS_FIGHTER].color = TCOD_white;

    game->class_info[CLASS_RANGER].name = "Ranger";
    game->class_info[CLASS_RANGER].color = TCOD_dark_green;

    game->class_info[CLASS_WIZARD].name = "Wizard";
    game->class_info[CLASS_WIZARD].color = TCOD_azure;

    game->class_info[CLASS_CLERIC].name = "Cleric";
    game->class_info[CLASS_CLERIC].color = TCOD_yellow;

    game->item_common.__placeholder = 0;

    game->item_info[ITEM_BOW].name = "Bow";
    game->item_info[ITEM_BOW].glyph = '}';
    game->item_info[ITEM_BOW].color = TCOD_white;

    game->item_info[ITEM_POTION].name = "Potion";
    game->item_info[ITEM_POTION].glyph = '!';
    game->item_info[ITEM_POTION].color = TCOD_white;

    game->item_info[ITEM_SCROLL].name = "Scroll";
    game->item_info[ITEM_SCROLL].glyph = '?';
    game->item_info[ITEM_SCROLL].color = TCOD_white;

    game->item_info[ITEM_SHIELD].name = "Shield";
    game->item_info[ITEM_SHIELD].glyph = ')';
    game->item_info[ITEM_SHIELD].color = TCOD_white;

    game->item_info[ITEM_SPEAR].name = "Spear";
    game->item_info[ITEM_SPEAR].glyph = '/';
    game->item_info[ITEM_SPEAR].color = TCOD_white;

    game->item_info[ITEM_SWORD].name = "Sword";
    game->item_info[ITEM_SWORD].glyph = '|';
    game->item_info[ITEM_SWORD].color = TCOD_white;

    for (int level = 0; level < NUM_MAPS; level++)
    {
        struct map *map = &game->maps[level];

        map_init(map, game, level);
    }

    game->messages = TCOD_list_new();

    game->player = NULL;

    game->turn = 0;
    game->turn_available = true;
    game->should_update = true;
    game->should_restart = false;
    game->should_quit = false;
    game->game_over = false;

    if (TCOD_sys_file_exists(SAVE_PATH))
    {
        game_load(game);
    }
    else
    {
        game_new(game);
    }

    return game;
}

void game_new(struct game *game)
{
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

        game->player = actor_create(game, RACE_HUMAN, CLASS_FIGHTER, FACTION_GOOD, level, x, y);
        game->player->glow = false;

        TCOD_list_push(map->actors, game->player);
        TCOD_list_push(tile->actors, game->player);

        game_log(
            game,
            level,
            x,
            y,
            TCOD_white,
            "Hail, %s %s!",
            game->race_info[game->player->race].name,
            game->class_info[game->player->class].name);
    }
}

void game_save(struct game *game)
{
    (void)game;
}

void game_load(struct game *game)
{
    (void)game;
}

void game_update(struct game *game)
{
    game->turn_available = true;

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

                TCOD_list_remove(tile->objects, object);
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

void game_log(struct game *game, int level, int x, int y, TCOD_color_t color, char *fmt, ...)
{
    if (level != game->player->level ||
        !game->player->fov ||
        !TCOD_map_is_in_fov(game->player->fov, x, y))
    {
        return;
    }

    char buffer[128];

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
    } while (line_end);
}

void game_destroy(struct game *game)
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
