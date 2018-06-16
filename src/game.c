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

    game->object_info[OBJECT_ALTAR].name = "Altar";
    game->object_info[OBJECT_ALTAR].glyph = '_';
    game->object_info[OBJECT_ALTAR].is_transparent = true;
    game->object_info[OBJECT_ALTAR].is_walkable = false;
    game->object_info[OBJECT_BRAZIER].name = "Brazier";
    game->object_info[OBJECT_BRAZIER].glyph = '*';
    game->object_info[OBJECT_BRAZIER].is_transparent = true;
    game->object_info[OBJECT_BRAZIER].is_walkable = false;
    game->object_info[OBJECT_DOOR_CLOSED].name = "Closed Door";
    game->object_info[OBJECT_DOOR_CLOSED].glyph = '+';
    game->object_info[OBJECT_DOOR_CLOSED].is_transparent = false;
    game->object_info[OBJECT_DOOR_CLOSED].is_walkable = false;
    game->object_info[OBJECT_DOOR_OPEN].name = "Open Door";
    game->object_info[OBJECT_DOOR_OPEN].glyph = '-';
    game->object_info[OBJECT_DOOR_OPEN].is_transparent = true;
    game->object_info[OBJECT_DOOR_OPEN].is_walkable = true;
    game->object_info[OBJECT_FOUNTAIN].name = "Fountain";
    game->object_info[OBJECT_FOUNTAIN].glyph = '{';
    game->object_info[OBJECT_FOUNTAIN].is_transparent = true;
    game->object_info[OBJECT_FOUNTAIN].is_walkable = false;
    game->object_info[OBJECT_STAIR_DOWN].name = "Stair Down";
    game->object_info[OBJECT_STAIR_DOWN].glyph = '>';
    game->object_info[OBJECT_STAIR_DOWN].is_transparent = true;
    game->object_info[OBJECT_STAIR_DOWN].is_walkable = true;
    game->object_info[OBJECT_STAIR_UP].name = "Stair Up";
    game->object_info[OBJECT_STAIR_UP].glyph = '<';
    game->object_info[OBJECT_STAIR_UP].is_transparent = true;
    game->object_info[OBJECT_STAIR_UP].is_walkable = true;
    game->object_info[OBJECT_THRONE].name = "Throne";
    game->object_info[OBJECT_THRONE].glyph = '\\';
    game->object_info[OBJECT_THRONE].is_transparent = true;
    game->object_info[OBJECT_THRONE].is_walkable = false;

    game->actor_common.glow_radius = 5;
    game->actor_common.glow_color = TCOD_white;
    game->actor_common.torch_radius = 10;
    game->actor_common.torch_color = TCOD_light_amber;

    game->race_info[RACE_DWARF].name = "Dwarf";
    game->race_info[RACE_DWARF].glyph = '@';
    game->race_info[RACE_DWARF].energy_per_turn = 0.5f;
    game->race_info[RACE_ELF].name = "Elf";
    game->race_info[RACE_ELF].glyph = '@';
    game->race_info[RACE_ELF].energy_per_turn = 0.5f;
    game->race_info[RACE_GNOME].name = "Gnome";
    game->race_info[RACE_GNOME].glyph = '@';
    game->race_info[RACE_GNOME].energy_per_turn = 0.5f;
    game->race_info[RACE_HALF_ELF].name = "Half-Elf";
    game->race_info[RACE_HALF_ELF].glyph = '@';
    game->race_info[RACE_HALF_ELF].energy_per_turn = 0.5f;
    game->race_info[RACE_HALF_ORC].name = "Half-Orc";
    game->race_info[RACE_HALF_ORC].glyph = '@';
    game->race_info[RACE_HALF_ORC].energy_per_turn = 0.5f;
    game->race_info[RACE_HALFLING].name = "Halfling";
    game->race_info[RACE_HALFLING].glyph = '@';
    game->race_info[RACE_HALFLING].energy_per_turn = 0.5f;
    game->race_info[RACE_HUMAN].name = "Human";
    game->race_info[RACE_HUMAN].glyph = '@';
    game->race_info[RACE_HUMAN].energy_per_turn = 0.5f;
    game->race_info[RACE_ABBERATION].name = "Abberation";
    game->race_info[RACE_ABBERATION].glyph = 'A';
    game->race_info[RACE_ABBERATION].energy_per_turn = 0.5f;
    game->race_info[RACE_ANIMAL].name = "Animal";
    game->race_info[RACE_ANIMAL].glyph = 'a';
    game->race_info[RACE_ANIMAL].energy_per_turn = 0.5f;
    game->race_info[RACE_BEAST].name = "Beast";
    game->race_info[RACE_BEAST].glyph = 'b';
    game->race_info[RACE_BEAST].energy_per_turn = 0.5f;
    game->race_info[RACE_CONSTRUCT].name = "Construct";
    game->race_info[RACE_CONSTRUCT].glyph = 'c';
    game->race_info[RACE_CONSTRUCT].energy_per_turn = 0.5f;
    game->race_info[RACE_DRAGON].name = "Dragon";
    game->race_info[RACE_DRAGON].glyph = 'D';
    game->race_info[RACE_DRAGON].energy_per_turn = 0.5f;
    game->race_info[RACE_ELEMENTAL].name = "Elemental";
    game->race_info[RACE_ELEMENTAL].glyph = 'e';
    game->race_info[RACE_ELEMENTAL].energy_per_turn = 0.5f;
    game->race_info[RACE_FEY].name = "Fey";
    game->race_info[RACE_FEY].glyph = 'f';
    game->race_info[RACE_FEY].energy_per_turn = 0.5f;
    game->race_info[RACE_GIANT].name = "Giant";
    game->race_info[RACE_GIANT].glyph = 'G';
    game->race_info[RACE_GIANT].energy_per_turn = 0.5f;
    game->race_info[RACE_GOBLINOID].name = "Goblinoid";
    game->race_info[RACE_GOBLINOID].glyph = 'g';
    game->race_info[RACE_GOBLINOID].energy_per_turn = 0.5f;
    game->race_info[RACE_MAGICAL_BEAST].name = "Magical Beast";
    game->race_info[RACE_MAGICAL_BEAST].glyph = 'M';
    game->race_info[RACE_MAGICAL_BEAST].energy_per_turn = 0.5f;
    game->race_info[RACE_MONSTROUS_HUMANOID].name = "Monstrous Humanoid";
    game->race_info[RACE_MONSTROUS_HUMANOID].glyph = 'm';
    game->race_info[RACE_MONSTROUS_HUMANOID].energy_per_turn = 0.5f;
    game->race_info[RACE_OOZE].name = "Ooze";
    game->race_info[RACE_OOZE].glyph = 'O';
    game->race_info[RACE_OOZE].energy_per_turn = 0.5f;
    game->race_info[RACE_ORC].name = "Orc";
    game->race_info[RACE_ORC].glyph = 'o';
    game->race_info[RACE_ORC].energy_per_turn = 0.5f;
    game->race_info[RACE_OUTSIDER].name = "Outsider";
    game->race_info[RACE_OUTSIDER].glyph = 'z';
    game->race_info[RACE_OUTSIDER].energy_per_turn = 0.5f;
    game->race_info[RACE_REPTILLIAN].name = "Reptillian";
    game->race_info[RACE_REPTILLIAN].glyph = 'r';
    game->race_info[RACE_REPTILLIAN].energy_per_turn = 0.5f;
    game->race_info[RACE_SHAPECHANGER].name = "Shapechanger";
    game->race_info[RACE_SHAPECHANGER].glyph = 's';
    game->race_info[RACE_SHAPECHANGER].energy_per_turn = 0.5f;
    game->race_info[RACE_UNDEAD].name = "Undead";
    game->race_info[RACE_UNDEAD].glyph = 'u';
    game->race_info[RACE_UNDEAD].energy_per_turn = 0.5f;
    game->race_info[RACE_VERMIN].name = "Vermin";
    game->race_info[RACE_VERMIN].glyph = 'v';
    game->race_info[RACE_VERMIN].energy_per_turn = 0.5f;

    game->class_info[CLASS_BARBARIAN].name = "Barbarian";
    game->class_info[CLASS_BARBARIAN].color = TCOD_amber;
    game->class_info[CLASS_BARBARIAN].hit_die = 10;
    game->class_info[CLASS_BARD].name = "Bard";
    game->class_info[CLASS_BARD].color = TCOD_purple;
    game->class_info[CLASS_BARD].hit_die = 10;
    game->class_info[CLASS_CLERIC].name = "Cleric";
    game->class_info[CLASS_CLERIC].color = TCOD_white;
    game->class_info[CLASS_CLERIC].hit_die = 10;
    game->class_info[CLASS_DRUID].name = "Druid";
    game->class_info[CLASS_DRUID].color = TCOD_orange;
    game->class_info[CLASS_DRUID].hit_die = 10;
    game->class_info[CLASS_FIGHTER].name = "Fighter";
    game->class_info[CLASS_FIGHTER].color = TCOD_brass;
    game->class_info[CLASS_FIGHTER].hit_die = 10;
    game->class_info[CLASS_MONK].name = "Monk";
    game->class_info[CLASS_MONK].color = TCOD_sea;
    game->class_info[CLASS_MONK].hit_die = 10;
    game->class_info[CLASS_PALADIN].name = "Paladin";
    game->class_info[CLASS_PALADIN].color = TCOD_pink;
    game->class_info[CLASS_PALADIN].hit_die = 10;
    game->class_info[CLASS_RANGER].name = "Ranger";
    game->class_info[CLASS_RANGER].color = TCOD_green;
    game->class_info[CLASS_RANGER].hit_die = 10;
    game->class_info[CLASS_ROGUE].name = "Rogue";
    game->class_info[CLASS_ROGUE].color = TCOD_yellow;
    game->class_info[CLASS_ROGUE].hit_die = 10;
    game->class_info[CLASS_SORCERER].name = "Sorcerer";
    game->class_info[CLASS_SORCERER].color = TCOD_flame;
    game->class_info[CLASS_SORCERER].hit_die = 10;
    game->class_info[CLASS_WIZARD].name = "Wizard";
    game->class_info[CLASS_WIZARD].color = TCOD_azure;
    game->class_info[CLASS_WIZARD].hit_die = 10;
    game->class_info[CLASS_ABBERATION].name = "Abberation";
    game->class_info[CLASS_ABBERATION].color = TCOD_grey;
    game->class_info[CLASS_ABBERATION].hit_die = 10;
    game->class_info[CLASS_ANIMAL].name = "Animal";
    game->class_info[CLASS_ANIMAL].color = TCOD_grey;
    game->class_info[CLASS_ANIMAL].hit_die = 10;
    game->class_info[CLASS_BEAST].name = "Beast";
    game->class_info[CLASS_BEAST].color = TCOD_grey;
    game->class_info[CLASS_BEAST].hit_die = 10;
    game->class_info[CLASS_COMMONER].name = "Commoner";
    game->class_info[CLASS_COMMONER].color = TCOD_grey;
    game->class_info[CLASS_COMMONER].hit_die = 10;
    game->class_info[CLASS_CONSTRUCT].name = "Construct";
    game->class_info[CLASS_CONSTRUCT].color = TCOD_grey;
    game->class_info[CLASS_CONSTRUCT].hit_die = 10;
    game->class_info[CLASS_DRAGON].name = "Dragon";
    game->class_info[CLASS_DRAGON].color = TCOD_grey;
    game->class_info[CLASS_DRAGON].hit_die = 10;
    game->class_info[CLASS_ELEMENTAL].name = "Elemental";
    game->class_info[CLASS_ELEMENTAL].color = TCOD_grey;
    game->class_info[CLASS_ELEMENTAL].hit_die = 10;
    game->class_info[CLASS_FEY].name = "Fey";
    game->class_info[CLASS_FEY].color = TCOD_grey;
    game->class_info[CLASS_FEY].hit_die = 10;
    game->class_info[CLASS_GIANT].name = "Giant";
    game->class_info[CLASS_GIANT].color = TCOD_grey;
    game->class_info[CLASS_GIANT].hit_die = 10;
    game->class_info[CLASS_HUMANOID].name = "Humanoid";
    game->class_info[CLASS_HUMANOID].color = TCOD_grey;
    game->class_info[CLASS_HUMANOID].hit_die = 10;
    game->class_info[CLASS_MAGICAL_BEAST].name = "Magical Beast";
    game->class_info[CLASS_MAGICAL_BEAST].color = TCOD_grey;
    game->class_info[CLASS_MAGICAL_BEAST].hit_die = 10;
    game->class_info[CLASS_MONSTROUS].name = "Monstrous";
    game->class_info[CLASS_MONSTROUS].color = TCOD_grey;
    game->class_info[CLASS_MONSTROUS].hit_die = 10;
    game->class_info[CLASS_OOZE].name = "Ooze";
    game->class_info[CLASS_OOZE].color = TCOD_grey;
    game->class_info[CLASS_OOZE].hit_die = 10;
    game->class_info[CLASS_OUTSIDER].name = "Outsider";
    game->class_info[CLASS_OUTSIDER].color = TCOD_grey;
    game->class_info[CLASS_OUTSIDER].hit_die = 10;
    game->class_info[CLASS_SHAPESHIFTER].name = "Shapeshifter";
    game->class_info[CLASS_SHAPESHIFTER].color = TCOD_grey;
    game->class_info[CLASS_SHAPESHIFTER].hit_die = 10;
    game->class_info[CLASS_UNDEAD].name = "Undead";
    game->class_info[CLASS_UNDEAD].color = TCOD_grey;
    game->class_info[CLASS_UNDEAD].hit_die = 10;
    game->class_info[CLASS_VERMIN].name = "Vermin";
    game->class_info[CLASS_VERMIN].color = TCOD_grey;
    game->class_info[CLASS_VERMIN].hit_die = 10;

    game->monster_prototypes[MONSTER_BASILISK].name = "Basilisk";
    game->monster_prototypes[MONSTER_BASILISK].race = RACE_BEAST;
    game->monster_prototypes[MONSTER_BASILISK].class = CLASS_BEAST;
    game->monster_prototypes[MONSTER_BEHOLDER].name = "Beholder";
    game->monster_prototypes[MONSTER_BEHOLDER].race = RACE_ABBERATION;
    game->monster_prototypes[MONSTER_BEHOLDER].class = CLASS_ABBERATION;
    game->monster_prototypes[MONSTER_BUGBEAR].name = "Bugbear";
    game->monster_prototypes[MONSTER_BUGBEAR].race = RACE_GOBLINOID;
    game->monster_prototypes[MONSTER_BUGBEAR].class = CLASS_HUMANOID;
    game->monster_prototypes[MONSTER_FIRE_ELEMENTAL].name = "Fire Elemental";
    game->monster_prototypes[MONSTER_FIRE_ELEMENTAL].race = RACE_ELEMENTAL;
    game->monster_prototypes[MONSTER_FIRE_ELEMENTAL].class = CLASS_ELEMENTAL;
    game->monster_prototypes[MONSTER_GELATINOUS_CUBE].name = "Gelatinous Cube";
    game->monster_prototypes[MONSTER_GELATINOUS_CUBE].race = RACE_OOZE;
    game->monster_prototypes[MONSTER_GELATINOUS_CUBE].class = CLASS_OOZE;
    game->monster_prototypes[MONSTER_GOBLIN].name = "Goblin";
    game->monster_prototypes[MONSTER_GOBLIN].race = RACE_GOBLINOID;
    game->monster_prototypes[MONSTER_GOBLIN].class = CLASS_HUMANOID;
    game->monster_prototypes[MONSTER_IRON_GOLEM].name = "Iron Golem";
    game->monster_prototypes[MONSTER_IRON_GOLEM].race = RACE_CONSTRUCT;
    game->monster_prototypes[MONSTER_IRON_GOLEM].class = CLASS_CONSTRUCT;
    game->monster_prototypes[MONSTER_OGRE].name = "Ogre";
    game->monster_prototypes[MONSTER_OGRE].race = RACE_GIANT;
    game->monster_prototypes[MONSTER_OGRE].class = CLASS_GIANT;
    game->monster_prototypes[MONSTER_ORC_CLERIC].name = "Orc Cleric";
    game->monster_prototypes[MONSTER_ORC_CLERIC].race = RACE_ORC;
    game->monster_prototypes[MONSTER_ORC_CLERIC].class = CLASS_CLERIC;
    game->monster_prototypes[MONSTER_ORC_FIGHTER].name = "Orc Fighter";
    game->monster_prototypes[MONSTER_ORC_FIGHTER].race = RACE_ORC;
    game->monster_prototypes[MONSTER_ORC_FIGHTER].class = CLASS_FIGHTER;
    game->monster_prototypes[MONSTER_ORC_RANGER].name = "Orc Ranger";
    game->monster_prototypes[MONSTER_ORC_RANGER].race = RACE_ORC;
    game->monster_prototypes[MONSTER_ORC_RANGER].class = CLASS_RANGER;
    game->monster_prototypes[MONSTER_PIXIE].name = "Pixie";
    game->monster_prototypes[MONSTER_PIXIE].race = RACE_FEY;
    game->monster_prototypes[MONSTER_PIXIE].class = CLASS_FEY;
    game->monster_prototypes[MONSTER_RAKSHASA].name = "Rakshasa";
    game->monster_prototypes[MONSTER_RAKSHASA].race = RACE_REPTILLIAN;
    game->monster_prototypes[MONSTER_RAKSHASA].class = CLASS_SHAPESHIFTER;
    game->monster_prototypes[MONSTER_RAT].name = "Rat";
    game->monster_prototypes[MONSTER_RAT].race = RACE_VERMIN;
    game->monster_prototypes[MONSTER_RAT].class = CLASS_VERMIN;
    game->monster_prototypes[MONSTER_RED_SLAAD].name = "Red Slaad";
    game->monster_prototypes[MONSTER_RED_SLAAD].race = RACE_OUTSIDER;
    game->monster_prototypes[MONSTER_RED_SLAAD].class = CLASS_OUTSIDER;
    game->monster_prototypes[MONSTER_WOLF].name = "Wolf";
    game->monster_prototypes[MONSTER_WOLF].race = RACE_ANIMAL;
    game->monster_prototypes[MONSTER_WOLF].class = CLASS_ANIMAL;
    game->monster_prototypes[MONSTER_WYRMLING].name = "Wyrmling";
    game->monster_prototypes[MONSTER_WYRMLING].race = RACE_DRAGON;
    game->monster_prototypes[MONSTER_WYRMLING].class = CLASS_DRAGON;
    game->monster_prototypes[MONSTER_ZOMBIE].name = "Zombie";
    game->monster_prototypes[MONSTER_ZOMBIE].race = RACE_UNDEAD;
    game->monster_prototypes[MONSTER_ZOMBIE].class = CLASS_UNDEAD;

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

        game->player = actor_create(game, "Blinky", RACE_HUMAN, CLASS_FIGHTER, FACTION_GOOD, level, x, y);
        game->player->glow = false;

        TCOD_list_push(map->actors, game->player);
        TCOD_list_push(tile->actors, game->player);

        game_log(
            game,
            level,
            x,
            y,
            TCOD_white,
            "Hail, %s!",
            game->player->name);
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

    char buffer[256];

    va_list args;
    va_start(args, fmt);
    vsprintf(buffer, fmt, args);
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
