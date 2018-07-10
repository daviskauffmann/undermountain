#include <libtcod/libtcod.h>
#include <malloc.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include "actor.h"
#include "game.h"
#include "input.h"
#include "map.h"
#include "message.h"
#include "projectile.h"
#include "window.h"

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

    game->actor_common.turns_to_chase = 10;
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
    game->class_info[CLASS_ABBERATION].color = TCOD_lightest_grey;
    game->class_info[CLASS_ABBERATION].hit_die = 10;
    game->class_info[CLASS_ANIMAL].name = "Animal";
    game->class_info[CLASS_ANIMAL].color = TCOD_lightest_grey;
    game->class_info[CLASS_ANIMAL].hit_die = 10;
    game->class_info[CLASS_BEAST].name = "Beast";
    game->class_info[CLASS_BEAST].color = TCOD_lightest_grey;
    game->class_info[CLASS_BEAST].hit_die = 10;
    game->class_info[CLASS_COMMONER].name = "Commoner";
    game->class_info[CLASS_COMMONER].color = TCOD_lightest_grey;
    game->class_info[CLASS_COMMONER].hit_die = 10;
    game->class_info[CLASS_CONSTRUCT].name = "Construct";
    game->class_info[CLASS_CONSTRUCT].color = TCOD_lightest_grey;
    game->class_info[CLASS_CONSTRUCT].hit_die = 10;
    game->class_info[CLASS_DRAGON].name = "Dragon";
    game->class_info[CLASS_DRAGON].color = TCOD_lightest_grey;
    game->class_info[CLASS_DRAGON].hit_die = 10;
    game->class_info[CLASS_ELEMENTAL].name = "Elemental";
    game->class_info[CLASS_ELEMENTAL].color = TCOD_lightest_grey;
    game->class_info[CLASS_ELEMENTAL].hit_die = 10;
    game->class_info[CLASS_FEY].name = "Fey";
    game->class_info[CLASS_FEY].color = TCOD_lightest_grey;
    game->class_info[CLASS_FEY].hit_die = 10;
    game->class_info[CLASS_GIANT].name = "Giant";
    game->class_info[CLASS_GIANT].color = TCOD_lightest_grey;
    game->class_info[CLASS_GIANT].hit_die = 10;
    game->class_info[CLASS_HUMANOID].name = "Humanoid";
    game->class_info[CLASS_HUMANOID].color = TCOD_lightest_grey;
    game->class_info[CLASS_HUMANOID].hit_die = 10;
    game->class_info[CLASS_MAGICAL_BEAST].name = "Magical Beast";
    game->class_info[CLASS_MAGICAL_BEAST].color = TCOD_lightest_grey;
    game->class_info[CLASS_MAGICAL_BEAST].hit_die = 10;
    game->class_info[CLASS_MONSTROUS].name = "Monstrous";
    game->class_info[CLASS_MONSTROUS].color = TCOD_lightest_grey;
    game->class_info[CLASS_MONSTROUS].hit_die = 10;
    game->class_info[CLASS_OOZE].name = "Ooze";
    game->class_info[CLASS_OOZE].color = TCOD_lightest_grey;
    game->class_info[CLASS_OOZE].hit_die = 10;
    game->class_info[CLASS_OUTSIDER].name = "Outsider";
    game->class_info[CLASS_OUTSIDER].color = TCOD_lightest_grey;
    game->class_info[CLASS_OUTSIDER].hit_die = 10;
    game->class_info[CLASS_SHAPESHIFTER].name = "Shapeshifter";
    game->class_info[CLASS_SHAPESHIFTER].color = TCOD_lightest_grey;
    game->class_info[CLASS_SHAPESHIFTER].hit_die = 10;
    game->class_info[CLASS_UNDEAD].name = "Undead";
    game->class_info[CLASS_UNDEAD].color = TCOD_lightest_grey;
    game->class_info[CLASS_UNDEAD].hit_die = 10;
    game->class_info[CLASS_VERMIN].name = "Vermin";
    game->class_info[CLASS_VERMIN].color = TCOD_lightest_grey;
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

    game->base_item_info[BASE_ITEM_BATTLEAXE].glyph = 'T';
    game->base_item_info[BASE_ITEM_BATTLEAXE].color = TCOD_white;
    game->base_item_info[BASE_ITEM_BATTLEAXE].weight = 7;
    game->base_item_info[BASE_ITEM_BATTLEAXE].equip_slot = EQUIP_SLOT_MAIN_HAND;
    game->base_item_info[BASE_ITEM_BATTLEAXE].weapon_damage = WEAPON_DAMAGE_SLASHING;
    game->base_item_info[BASE_ITEM_BATTLEAXE].weapon_size = WEAPON_SIZE_MEDIUM;
    game->base_item_info[BASE_ITEM_BATTLEAXE].ranged = false;
    game->base_item_info[BASE_ITEM_BATTLEAXE].num_dice = 1;
    game->base_item_info[BASE_ITEM_BATTLEAXE].die_to_roll = 8;
    game->base_item_info[BASE_ITEM_BATTLEAXE].crit_threat = 20;
    game->base_item_info[BASE_ITEM_BATTLEAXE].crit_mult = 3;
    game->base_item_info[BASE_ITEM_BATTLEAXE].base_cost = 10;
    game->base_item_info[BASE_ITEM_BATTLEAXE].stack = 1;
    game->base_item_info[BASE_ITEM_BATTLEAXE].base_armor_class = 0;
    game->base_item_info[BASE_ITEM_BATTLEAXE].armor_check_penalty = 0;
    game->base_item_info[BASE_ITEM_BATTLEAXE].arcane_spell_failure = 0;
    game->base_item_info[BASE_ITEM_CLUB].glyph = '!';
    game->base_item_info[BASE_ITEM_CLUB].color = TCOD_white;
    game->base_item_info[BASE_ITEM_CLUB].weight = 1;
    game->base_item_info[BASE_ITEM_CLUB].equip_slot = EQUIP_SLOT_MAIN_HAND;
    game->base_item_info[BASE_ITEM_CLUB].weapon_damage = WEAPON_DAMAGE_BLUDGEONING;
    game->base_item_info[BASE_ITEM_CLUB].weapon_size = WEAPON_SIZE_MEDIUM;
    game->base_item_info[BASE_ITEM_CLUB].ranged = false;
    game->base_item_info[BASE_ITEM_CLUB].num_dice = 1;
    game->base_item_info[BASE_ITEM_CLUB].die_to_roll = 6;
    game->base_item_info[BASE_ITEM_CLUB].crit_threat = 20;
    game->base_item_info[BASE_ITEM_CLUB].crit_mult = 2;
    game->base_item_info[BASE_ITEM_CLUB].base_cost = 3;
    game->base_item_info[BASE_ITEM_CLUB].stack = 1;
    game->base_item_info[BASE_ITEM_CLUB].base_armor_class = 0;
    game->base_item_info[BASE_ITEM_CLUB].armor_check_penalty = 0;
    game->base_item_info[BASE_ITEM_CLUB].arcane_spell_failure = 0;
    game->base_item_info[BASE_ITEM_DAGGER].glyph = '-';
    game->base_item_info[BASE_ITEM_DAGGER].color = TCOD_white;
    game->base_item_info[BASE_ITEM_DAGGER].weight = 1;
    game->base_item_info[BASE_ITEM_DAGGER].equip_slot = EQUIP_SLOT_MAIN_HAND;
    game->base_item_info[BASE_ITEM_DAGGER].weapon_damage = WEAPON_DAMAGE_PIERCING;
    game->base_item_info[BASE_ITEM_DAGGER].weapon_size = WEAPON_SIZE_TINY;
    game->base_item_info[BASE_ITEM_DAGGER].ranged = false;
    game->base_item_info[BASE_ITEM_DAGGER].num_dice = 1;
    game->base_item_info[BASE_ITEM_DAGGER].die_to_roll = 4;
    game->base_item_info[BASE_ITEM_DAGGER].crit_threat = 19;
    game->base_item_info[BASE_ITEM_DAGGER].crit_mult = 2;
    game->base_item_info[BASE_ITEM_DAGGER].base_cost = 4;
    game->base_item_info[BASE_ITEM_DAGGER].stack = 1;
    game->base_item_info[BASE_ITEM_DAGGER].base_armor_class = 0;
    game->base_item_info[BASE_ITEM_DAGGER].armor_check_penalty = 0;
    game->base_item_info[BASE_ITEM_DAGGER].arcane_spell_failure = 0;
    game->base_item_info[BASE_ITEM_GREATAXE].glyph = 'T';
    game->base_item_info[BASE_ITEM_GREATAXE].color = TCOD_white;
    game->base_item_info[BASE_ITEM_GREATAXE].weight = 20;
    game->base_item_info[BASE_ITEM_GREATAXE].equip_slot = EQUIP_SLOT_MAIN_HAND;
    game->base_item_info[BASE_ITEM_GREATAXE].weapon_damage = WEAPON_DAMAGE_SLASHING;
    game->base_item_info[BASE_ITEM_GREATAXE].weapon_size = WEAPON_SIZE_LARGE;
    game->base_item_info[BASE_ITEM_GREATAXE].ranged = false;
    game->base_item_info[BASE_ITEM_GREATAXE].num_dice = 1;
    game->base_item_info[BASE_ITEM_GREATAXE].die_to_roll = 12;
    game->base_item_info[BASE_ITEM_GREATAXE].crit_threat = 20;
    game->base_item_info[BASE_ITEM_GREATAXE].crit_mult = 3;
    game->base_item_info[BASE_ITEM_GREATAXE].base_cost = 20;
    game->base_item_info[BASE_ITEM_GREATAXE].stack = 1;
    game->base_item_info[BASE_ITEM_GREATAXE].base_armor_class = 0;
    game->base_item_info[BASE_ITEM_GREATAXE].armor_check_penalty = 0;
    game->base_item_info[BASE_ITEM_GREATAXE].arcane_spell_failure = 0;
    game->base_item_info[BASE_ITEM_GREATSWORD].glyph = '|';
    game->base_item_info[BASE_ITEM_GREATSWORD].color = TCOD_white;
    game->base_item_info[BASE_ITEM_GREATSWORD].weight = 15;
    game->base_item_info[BASE_ITEM_GREATSWORD].equip_slot = EQUIP_SLOT_MAIN_HAND;
    game->base_item_info[BASE_ITEM_GREATSWORD].weapon_damage = WEAPON_DAMAGE_SLASHING;
    game->base_item_info[BASE_ITEM_GREATSWORD].weapon_size = WEAPON_SIZE_LARGE;
    game->base_item_info[BASE_ITEM_GREATSWORD].ranged = false;
    game->base_item_info[BASE_ITEM_GREATSWORD].num_dice = 2;
    game->base_item_info[BASE_ITEM_GREATSWORD].die_to_roll = 6;
    game->base_item_info[BASE_ITEM_GREATSWORD].crit_threat = 19;
    game->base_item_info[BASE_ITEM_GREATSWORD].crit_mult = 2;
    game->base_item_info[BASE_ITEM_GREATSWORD].base_cost = 50;
    game->base_item_info[BASE_ITEM_GREATSWORD].stack = 1;
    game->base_item_info[BASE_ITEM_GREATSWORD].base_armor_class = 0;
    game->base_item_info[BASE_ITEM_GREATSWORD].armor_check_penalty = 0;
    game->base_item_info[BASE_ITEM_GREATSWORD].arcane_spell_failure = 0;
    game->base_item_info[BASE_ITEM_HALBERD].glyph = 'P';
    game->base_item_info[BASE_ITEM_HALBERD].color = TCOD_white;
    game->base_item_info[BASE_ITEM_HALBERD].weight = 15;
    game->base_item_info[BASE_ITEM_HALBERD].equip_slot = EQUIP_SLOT_MAIN_HAND;
    game->base_item_info[BASE_ITEM_HALBERD].weapon_damage = WEAPON_DAMAGE_PIERCING_AND_SLASHING;
    game->base_item_info[BASE_ITEM_HALBERD].weapon_size = WEAPON_SIZE_LARGE;
    game->base_item_info[BASE_ITEM_HALBERD].ranged = false;
    game->base_item_info[BASE_ITEM_HALBERD].num_dice = 1;
    game->base_item_info[BASE_ITEM_HALBERD].die_to_roll = 10;
    game->base_item_info[BASE_ITEM_HALBERD].crit_threat = 20;
    game->base_item_info[BASE_ITEM_HALBERD].crit_mult = 3;
    game->base_item_info[BASE_ITEM_HALBERD].base_cost = 10;
    game->base_item_info[BASE_ITEM_HALBERD].stack = 1;
    game->base_item_info[BASE_ITEM_HALBERD].base_armor_class = 0;
    game->base_item_info[BASE_ITEM_HALBERD].armor_check_penalty = 0;
    game->base_item_info[BASE_ITEM_HALBERD].arcane_spell_failure = 0;
    game->base_item_info[BASE_ITEM_HEAVY_CROSSBOW].glyph = 'T';
    game->base_item_info[BASE_ITEM_HEAVY_CROSSBOW].color = TCOD_white;
    game->base_item_info[BASE_ITEM_HEAVY_CROSSBOW].weight = 9;
    game->base_item_info[BASE_ITEM_HEAVY_CROSSBOW].equip_slot = EQUIP_SLOT_MAIN_HAND;
    game->base_item_info[BASE_ITEM_HEAVY_CROSSBOW].weapon_damage = WEAPON_DAMAGE_PIERCING;
    game->base_item_info[BASE_ITEM_HEAVY_CROSSBOW].weapon_size = WEAPON_SIZE_MEDIUM;
    game->base_item_info[BASE_ITEM_HEAVY_CROSSBOW].ranged = true;
    game->base_item_info[BASE_ITEM_HEAVY_CROSSBOW].num_dice = 1;
    game->base_item_info[BASE_ITEM_HEAVY_CROSSBOW].die_to_roll = 10;
    game->base_item_info[BASE_ITEM_HEAVY_CROSSBOW].crit_threat = 19;
    game->base_item_info[BASE_ITEM_HEAVY_CROSSBOW].crit_mult = 2;
    game->base_item_info[BASE_ITEM_HEAVY_CROSSBOW].base_cost = 50;
    game->base_item_info[BASE_ITEM_HEAVY_CROSSBOW].stack = 1;
    game->base_item_info[BASE_ITEM_HEAVY_CROSSBOW].base_armor_class = 0;
    game->base_item_info[BASE_ITEM_HEAVY_CROSSBOW].armor_check_penalty = 0;
    game->base_item_info[BASE_ITEM_HEAVY_CROSSBOW].arcane_spell_failure = 0;
    game->base_item_info[BASE_ITEM_KATANA].glyph = '(';
    game->base_item_info[BASE_ITEM_KATANA].color = TCOD_white;
    game->base_item_info[BASE_ITEM_KATANA].weight = 10;
    game->base_item_info[BASE_ITEM_KATANA].equip_slot = EQUIP_SLOT_MAIN_HAND;
    game->base_item_info[BASE_ITEM_KATANA].weapon_damage = WEAPON_DAMAGE_SLASHING;
    game->base_item_info[BASE_ITEM_KATANA].weapon_size = WEAPON_SIZE_MEDIUM;
    game->base_item_info[BASE_ITEM_KATANA].ranged = false;
    game->base_item_info[BASE_ITEM_KATANA].num_dice = 1;
    game->base_item_info[BASE_ITEM_KATANA].die_to_roll = 10;
    game->base_item_info[BASE_ITEM_KATANA].crit_threat = 19;
    game->base_item_info[BASE_ITEM_KATANA].crit_mult = 2;
    game->base_item_info[BASE_ITEM_KATANA].base_cost = 40;
    game->base_item_info[BASE_ITEM_KATANA].stack = 1;
    game->base_item_info[BASE_ITEM_KATANA].base_armor_class = 0;
    game->base_item_info[BASE_ITEM_KATANA].armor_check_penalty = 0;
    game->base_item_info[BASE_ITEM_KATANA].arcane_spell_failure = 0;
    game->base_item_info[BASE_ITEM_LARGE_SHIELD].glyph = ')';
    game->base_item_info[BASE_ITEM_LARGE_SHIELD].color = TCOD_white;
    game->base_item_info[BASE_ITEM_LARGE_SHIELD].weight = 15;
    game->base_item_info[BASE_ITEM_LARGE_SHIELD].equip_slot = EQUIP_SLOT_OFF_HAND;
    game->base_item_info[BASE_ITEM_LARGE_SHIELD].weapon_damage = 0;
    game->base_item_info[BASE_ITEM_LARGE_SHIELD].weapon_size = 0;
    game->base_item_info[BASE_ITEM_LARGE_SHIELD].ranged = false;
    game->base_item_info[BASE_ITEM_LARGE_SHIELD].num_dice = 0;
    game->base_item_info[BASE_ITEM_LARGE_SHIELD].die_to_roll = 0;
    game->base_item_info[BASE_ITEM_LARGE_SHIELD].crit_threat = 0;
    game->base_item_info[BASE_ITEM_LARGE_SHIELD].crit_mult = 0;
    game->base_item_info[BASE_ITEM_LARGE_SHIELD].base_cost = 50;
    game->base_item_info[BASE_ITEM_LARGE_SHIELD].stack = 1;
    game->base_item_info[BASE_ITEM_LARGE_SHIELD].base_armor_class = 2;
    game->base_item_info[BASE_ITEM_LARGE_SHIELD].armor_check_penalty = -2;
    game->base_item_info[BASE_ITEM_LARGE_SHIELD].arcane_spell_failure = 15;
    game->base_item_info[BASE_ITEM_LIGHT_CROSSBOW].glyph = 'T';
    game->base_item_info[BASE_ITEM_LIGHT_CROSSBOW].color = TCOD_white;
    game->base_item_info[BASE_ITEM_LIGHT_CROSSBOW].weight = 6;
    game->base_item_info[BASE_ITEM_LIGHT_CROSSBOW].equip_slot = EQUIP_SLOT_MAIN_HAND;
    game->base_item_info[BASE_ITEM_LIGHT_CROSSBOW].weapon_damage = WEAPON_DAMAGE_PIERCING;
    game->base_item_info[BASE_ITEM_LIGHT_CROSSBOW].weapon_size = WEAPON_SIZE_SMALL;
    game->base_item_info[BASE_ITEM_LIGHT_CROSSBOW].ranged = true;
    game->base_item_info[BASE_ITEM_LIGHT_CROSSBOW].num_dice = 1;
    game->base_item_info[BASE_ITEM_LIGHT_CROSSBOW].die_to_roll = 8;
    game->base_item_info[BASE_ITEM_LIGHT_CROSSBOW].crit_threat = 19;
    game->base_item_info[BASE_ITEM_LIGHT_CROSSBOW].crit_mult = 2;
    game->base_item_info[BASE_ITEM_LIGHT_CROSSBOW].base_cost = 35;
    game->base_item_info[BASE_ITEM_LIGHT_CROSSBOW].stack = 1;
    game->base_item_info[BASE_ITEM_LIGHT_CROSSBOW].base_armor_class = 0;
    game->base_item_info[BASE_ITEM_LIGHT_CROSSBOW].armor_check_penalty = 0;
    game->base_item_info[BASE_ITEM_LIGHT_CROSSBOW].arcane_spell_failure = 0;
    game->base_item_info[BASE_ITEM_LONGBOW].glyph = '}';
    game->base_item_info[BASE_ITEM_LONGBOW].color = TCOD_white;
    game->base_item_info[BASE_ITEM_LONGBOW].weight = 3;
    game->base_item_info[BASE_ITEM_LONGBOW].equip_slot = EQUIP_SLOT_MAIN_HAND;
    game->base_item_info[BASE_ITEM_LONGBOW].weapon_damage = WEAPON_DAMAGE_PIERCING;
    game->base_item_info[BASE_ITEM_LONGBOW].weapon_size = WEAPON_SIZE_LARGE;
    game->base_item_info[BASE_ITEM_LONGBOW].ranged = true;
    game->base_item_info[BASE_ITEM_LONGBOW].num_dice = 1;
    game->base_item_info[BASE_ITEM_LONGBOW].die_to_roll = 8;
    game->base_item_info[BASE_ITEM_LONGBOW].crit_threat = 20;
    game->base_item_info[BASE_ITEM_LONGBOW].crit_mult = 3;
    game->base_item_info[BASE_ITEM_LONGBOW].base_cost = 75;
    game->base_item_info[BASE_ITEM_LONGBOW].stack = 1;
    game->base_item_info[BASE_ITEM_LONGBOW].base_armor_class = 0;
    game->base_item_info[BASE_ITEM_LONGBOW].armor_check_penalty = 0;
    game->base_item_info[BASE_ITEM_LONGBOW].arcane_spell_failure = 0;
    game->base_item_info[BASE_ITEM_LONGSWORD].glyph = '|';
    game->base_item_info[BASE_ITEM_LONGSWORD].color = TCOD_white;
    game->base_item_info[BASE_ITEM_LONGSWORD].weight = 4;
    game->base_item_info[BASE_ITEM_LONGSWORD].equip_slot = EQUIP_SLOT_MAIN_HAND;
    game->base_item_info[BASE_ITEM_LONGSWORD].weapon_damage = WEAPON_DAMAGE_SLASHING;
    game->base_item_info[BASE_ITEM_LONGSWORD].weapon_size = WEAPON_SIZE_MEDIUM;
    game->base_item_info[BASE_ITEM_LONGSWORD].ranged = false;
    game->base_item_info[BASE_ITEM_LONGSWORD].num_dice = 1;
    game->base_item_info[BASE_ITEM_LONGSWORD].die_to_roll = 8;
    game->base_item_info[BASE_ITEM_LONGSWORD].crit_threat = 19;
    game->base_item_info[BASE_ITEM_LONGSWORD].crit_mult = 2;
    game->base_item_info[BASE_ITEM_LONGSWORD].base_cost = 15;
    game->base_item_info[BASE_ITEM_LONGSWORD].stack = 1;
    game->base_item_info[BASE_ITEM_LONGSWORD].base_armor_class = 0;
    game->base_item_info[BASE_ITEM_LONGSWORD].armor_check_penalty = 0;
    game->base_item_info[BASE_ITEM_LONGSWORD].arcane_spell_failure = 0;
    game->base_item_info[BASE_ITEM_MACE].glyph = '!';
    game->base_item_info[BASE_ITEM_MACE].color = TCOD_white;
    game->base_item_info[BASE_ITEM_MACE].weight = 6;
    game->base_item_info[BASE_ITEM_MACE].equip_slot = EQUIP_SLOT_MAIN_HAND;
    game->base_item_info[BASE_ITEM_MACE].weapon_damage = WEAPON_DAMAGE_BLUDGEONING;
    game->base_item_info[BASE_ITEM_MACE].weapon_size = WEAPON_SIZE_SMALL;
    game->base_item_info[BASE_ITEM_MACE].ranged = false;
    game->base_item_info[BASE_ITEM_MACE].num_dice = 1;
    game->base_item_info[BASE_ITEM_MACE].die_to_roll = 6;
    game->base_item_info[BASE_ITEM_MACE].crit_threat = 19;
    game->base_item_info[BASE_ITEM_MACE].crit_mult = 2;
    game->base_item_info[BASE_ITEM_MACE].base_cost = 5;
    game->base_item_info[BASE_ITEM_MACE].stack = 1;
    game->base_item_info[BASE_ITEM_MACE].base_armor_class = 0;
    game->base_item_info[BASE_ITEM_MACE].armor_check_penalty = 0;
    game->base_item_info[BASE_ITEM_MACE].arcane_spell_failure = 0;
    game->base_item_info[BASE_ITEM_QUARTERSTAFF].glyph = '|';
    game->base_item_info[BASE_ITEM_QUARTERSTAFF].color = TCOD_white;
    game->base_item_info[BASE_ITEM_QUARTERSTAFF].weight = 4;
    game->base_item_info[BASE_ITEM_QUARTERSTAFF].equip_slot = EQUIP_SLOT_MAIN_HAND;
    game->base_item_info[BASE_ITEM_QUARTERSTAFF].weapon_damage = WEAPON_DAMAGE_BLUDGEONING;
    game->base_item_info[BASE_ITEM_QUARTERSTAFF].weapon_size = WEAPON_SIZE_LARGE;
    game->base_item_info[BASE_ITEM_QUARTERSTAFF].ranged = false;
    game->base_item_info[BASE_ITEM_QUARTERSTAFF].num_dice = 1;
    game->base_item_info[BASE_ITEM_QUARTERSTAFF].die_to_roll = 6;
    game->base_item_info[BASE_ITEM_QUARTERSTAFF].crit_threat = 20;
    game->base_item_info[BASE_ITEM_QUARTERSTAFF].crit_mult = 2;
    game->base_item_info[BASE_ITEM_QUARTERSTAFF].base_cost = 1;
    game->base_item_info[BASE_ITEM_QUARTERSTAFF].stack = 1;
    game->base_item_info[BASE_ITEM_QUARTERSTAFF].base_armor_class = 0;
    game->base_item_info[BASE_ITEM_QUARTERSTAFF].armor_check_penalty = 0;
    game->base_item_info[BASE_ITEM_QUARTERSTAFF].arcane_spell_failure = 0;
    game->base_item_info[BASE_ITEM_SCIMITAR].glyph = '|';
    game->base_item_info[BASE_ITEM_SCIMITAR].color = TCOD_white;
    game->base_item_info[BASE_ITEM_SCIMITAR].weight = 4;
    game->base_item_info[BASE_ITEM_SCIMITAR].equip_slot = EQUIP_SLOT_MAIN_HAND;
    game->base_item_info[BASE_ITEM_SCIMITAR].weapon_damage = WEAPON_DAMAGE_SLASHING;
    game->base_item_info[BASE_ITEM_SCIMITAR].weapon_size = WEAPON_SIZE_MEDIUM;
    game->base_item_info[BASE_ITEM_SCIMITAR].ranged = false;
    game->base_item_info[BASE_ITEM_SCIMITAR].num_dice = 1;
    game->base_item_info[BASE_ITEM_SCIMITAR].die_to_roll = 6;
    game->base_item_info[BASE_ITEM_SCIMITAR].crit_threat = 18;
    game->base_item_info[BASE_ITEM_SCIMITAR].crit_mult = 2;
    game->base_item_info[BASE_ITEM_SCIMITAR].base_cost = 15;
    game->base_item_info[BASE_ITEM_SCIMITAR].stack = 1;
    game->base_item_info[BASE_ITEM_SCIMITAR].base_armor_class = 0;
    game->base_item_info[BASE_ITEM_SCIMITAR].armor_check_penalty = 0;
    game->base_item_info[BASE_ITEM_SCIMITAR].arcane_spell_failure = 0;
    game->base_item_info[BASE_ITEM_SHORTBOW].glyph = '}';
    game->base_item_info[BASE_ITEM_SHORTBOW].color = TCOD_white;
    game->base_item_info[BASE_ITEM_SHORTBOW].weight = 2;
    game->base_item_info[BASE_ITEM_SHORTBOW].equip_slot = EQUIP_SLOT_MAIN_HAND;
    game->base_item_info[BASE_ITEM_SHORTBOW].weapon_damage = WEAPON_DAMAGE_PIERCING;
    game->base_item_info[BASE_ITEM_SHORTBOW].weapon_size = WEAPON_SIZE_MEDIUM;
    game->base_item_info[BASE_ITEM_SHORTBOW].ranged = true;
    game->base_item_info[BASE_ITEM_SHORTBOW].num_dice = 1;
    game->base_item_info[BASE_ITEM_SHORTBOW].die_to_roll = 6;
    game->base_item_info[BASE_ITEM_SHORTBOW].crit_threat = 20;
    game->base_item_info[BASE_ITEM_SHORTBOW].crit_mult = 3;
    game->base_item_info[BASE_ITEM_SHORTBOW].base_cost = 30;
    game->base_item_info[BASE_ITEM_SHORTBOW].stack = 1;
    game->base_item_info[BASE_ITEM_SHORTBOW].base_armor_class = 0;
    game->base_item_info[BASE_ITEM_SHORTBOW].armor_check_penalty = 0;
    game->base_item_info[BASE_ITEM_SHORTBOW].arcane_spell_failure = 0;
    game->base_item_info[BASE_ITEM_SHORTSWORD].glyph = '-';
    game->base_item_info[BASE_ITEM_SHORTSWORD].color = TCOD_white;
    game->base_item_info[BASE_ITEM_SHORTSWORD].weight = 3;
    game->base_item_info[BASE_ITEM_SHORTSWORD].equip_slot = EQUIP_SLOT_MAIN_HAND;
    game->base_item_info[BASE_ITEM_SHORTSWORD].weapon_damage = WEAPON_DAMAGE_PIERCING;
    game->base_item_info[BASE_ITEM_SHORTSWORD].weapon_size = WEAPON_SIZE_SMALL;
    game->base_item_info[BASE_ITEM_SHORTSWORD].ranged = false;
    game->base_item_info[BASE_ITEM_SHORTSWORD].num_dice = 1;
    game->base_item_info[BASE_ITEM_SHORTSWORD].die_to_roll = 6;
    game->base_item_info[BASE_ITEM_SHORTSWORD].crit_threat = 19;
    game->base_item_info[BASE_ITEM_SHORTSWORD].crit_mult = 2;
    game->base_item_info[BASE_ITEM_SHORTSWORD].base_cost = 10;
    game->base_item_info[BASE_ITEM_SHORTSWORD].stack = 1;
    game->base_item_info[BASE_ITEM_SHORTSWORD].base_armor_class = 0;
    game->base_item_info[BASE_ITEM_SHORTSWORD].armor_check_penalty = 0;
    game->base_item_info[BASE_ITEM_SHORTSWORD].arcane_spell_failure = 0;
    game->base_item_info[BASE_ITEM_SMALL_SHIELD].glyph = ')';
    game->base_item_info[BASE_ITEM_SMALL_SHIELD].color = TCOD_white;
    game->base_item_info[BASE_ITEM_SMALL_SHIELD].weight = 6;
    game->base_item_info[BASE_ITEM_SMALL_SHIELD].equip_slot = EQUIP_SLOT_OFF_HAND;
    game->base_item_info[BASE_ITEM_SMALL_SHIELD].weapon_damage = 0;
    game->base_item_info[BASE_ITEM_SMALL_SHIELD].weapon_size = 0;
    game->base_item_info[BASE_ITEM_SMALL_SHIELD].ranged = false;
    game->base_item_info[BASE_ITEM_SMALL_SHIELD].num_dice = 0;
    game->base_item_info[BASE_ITEM_SMALL_SHIELD].die_to_roll = 0;
    game->base_item_info[BASE_ITEM_SMALL_SHIELD].crit_threat = 0;
    game->base_item_info[BASE_ITEM_SMALL_SHIELD].crit_mult = 0;
    game->base_item_info[BASE_ITEM_SMALL_SHIELD].base_cost = 9;
    game->base_item_info[BASE_ITEM_SMALL_SHIELD].stack = 1;
    game->base_item_info[BASE_ITEM_SMALL_SHIELD].base_armor_class = 1;
    game->base_item_info[BASE_ITEM_SMALL_SHIELD].armor_check_penalty = -1;
    game->base_item_info[BASE_ITEM_SMALL_SHIELD].arcane_spell_failure = 5;
    game->base_item_info[BASE_ITEM_SPEAR].glyph = '/';
    game->base_item_info[BASE_ITEM_SPEAR].color = TCOD_white;
    game->base_item_info[BASE_ITEM_SPEAR].weight = 3;
    game->base_item_info[BASE_ITEM_SPEAR].equip_slot = EQUIP_SLOT_MAIN_HAND;
    game->base_item_info[BASE_ITEM_SPEAR].weapon_damage = WEAPON_DAMAGE_PIERCING;
    game->base_item_info[BASE_ITEM_SPEAR].weapon_size = WEAPON_SIZE_LARGE;
    game->base_item_info[BASE_ITEM_SPEAR].ranged = false;
    game->base_item_info[BASE_ITEM_SPEAR].num_dice = 1;
    game->base_item_info[BASE_ITEM_SPEAR].die_to_roll = 8;
    game->base_item_info[BASE_ITEM_SPEAR].crit_threat = 20;
    game->base_item_info[BASE_ITEM_SPEAR].crit_mult = 3;
    game->base_item_info[BASE_ITEM_SPEAR].base_cost = 1;
    game->base_item_info[BASE_ITEM_SPEAR].stack = 1;
    game->base_item_info[BASE_ITEM_SPEAR].base_armor_class = 0;
    game->base_item_info[BASE_ITEM_SPEAR].armor_check_penalty = 0;
    game->base_item_info[BASE_ITEM_SPEAR].arcane_spell_failure = 0;
    game->base_item_info[BASE_ITEM_TOWER_SHIELD].glyph = ')';
    game->base_item_info[BASE_ITEM_TOWER_SHIELD].color = TCOD_white;
    game->base_item_info[BASE_ITEM_TOWER_SHIELD].weight = 45;
    game->base_item_info[BASE_ITEM_TOWER_SHIELD].equip_slot = EQUIP_SLOT_OFF_HAND;
    game->base_item_info[BASE_ITEM_TOWER_SHIELD].weapon_damage = 0;
    game->base_item_info[BASE_ITEM_TOWER_SHIELD].weapon_size = 0;
    game->base_item_info[BASE_ITEM_TOWER_SHIELD].ranged = false;
    game->base_item_info[BASE_ITEM_TOWER_SHIELD].num_dice = 0;
    game->base_item_info[BASE_ITEM_TOWER_SHIELD].die_to_roll = 0;
    game->base_item_info[BASE_ITEM_TOWER_SHIELD].crit_threat = 0;
    game->base_item_info[BASE_ITEM_TOWER_SHIELD].crit_mult = 0;
    game->base_item_info[BASE_ITEM_TOWER_SHIELD].base_cost = 100;
    game->base_item_info[BASE_ITEM_TOWER_SHIELD].stack = 1;
    game->base_item_info[BASE_ITEM_TOWER_SHIELD].base_armor_class = 3;
    game->base_item_info[BASE_ITEM_TOWER_SHIELD].armor_check_penalty = -10;
    game->base_item_info[BASE_ITEM_TOWER_SHIELD].arcane_spell_failure = 50;
    game->base_item_info[BASE_ITEM_WARHAMMER].glyph = '!';
    game->base_item_info[BASE_ITEM_WARHAMMER].color = TCOD_white;
    game->base_item_info[BASE_ITEM_WARHAMMER].weight = 8;
    game->base_item_info[BASE_ITEM_WARHAMMER].equip_slot = EQUIP_SLOT_MAIN_HAND;
    game->base_item_info[BASE_ITEM_WARHAMMER].weapon_damage = WEAPON_DAMAGE_BLUDGEONING;
    game->base_item_info[BASE_ITEM_WARHAMMER].weapon_size = WEAPON_SIZE_MEDIUM;
    game->base_item_info[BASE_ITEM_WARHAMMER].ranged = false;
    game->base_item_info[BASE_ITEM_WARHAMMER].num_dice = 1;
    game->base_item_info[BASE_ITEM_WARHAMMER].die_to_roll = 8;
    game->base_item_info[BASE_ITEM_WARHAMMER].crit_threat = 20;
    game->base_item_info[BASE_ITEM_WARHAMMER].crit_mult = 3;
    game->base_item_info[BASE_ITEM_WARHAMMER].base_cost = 12;
    game->base_item_info[BASE_ITEM_WARHAMMER].stack = 1;
    game->base_item_info[BASE_ITEM_WARHAMMER].base_armor_class = 0;
    game->base_item_info[BASE_ITEM_WARHAMMER].armor_check_penalty = 0;
    game->base_item_info[BASE_ITEM_WARHAMMER].arcane_spell_failure = 0;

    game->item_info[ITEM_BATTLEAXE].base_type = BASE_ITEM_BATTLEAXE;
    game->item_info[ITEM_BATTLEAXE].name = "Battleaxe";
    game->item_info[ITEM_BATTLEAXE].description = "A generic battleaxe.";
    game->item_info[ITEM_BATTLEAXE].enhancement_bonus = 0;
    game->item_info[ITEM_BATTLEAXE].armor_class_type = 0;
    game->item_info[ITEM_BATTLEAXE].armor_class_bonus = 0;
    game->item_info[ITEM_BATTLEAXE_1].base_type = BASE_ITEM_BATTLEAXE;
    game->item_info[ITEM_BATTLEAXE_1].name = "Battleaxe + 1";
    game->item_info[ITEM_BATTLEAXE_1].description = "A special battleaxe.";
    game->item_info[ITEM_BATTLEAXE_1].enhancement_bonus = 1;
    game->item_info[ITEM_BATTLEAXE_1].armor_class_type = 0;
    game->item_info[ITEM_BATTLEAXE_1].armor_class_bonus = 0;
    game->item_info[ITEM_CLUB].base_type = BASE_ITEM_CLUB;
    game->item_info[ITEM_CLUB].name = "Club";
    game->item_info[ITEM_CLUB].description = "A generic club.";
    game->item_info[ITEM_CLUB].enhancement_bonus = 0;
    game->item_info[ITEM_CLUB].armor_class_type = 0;
    game->item_info[ITEM_CLUB].armor_class_bonus = 0;
    game->item_info[ITEM_CLUB_1].base_type = BASE_ITEM_CLUB;
    game->item_info[ITEM_CLUB_1].name = "Club + 1";
    game->item_info[ITEM_CLUB_1].description = "A special club.";
    game->item_info[ITEM_CLUB_1].enhancement_bonus = 1;
    game->item_info[ITEM_CLUB_1].armor_class_type = 0;
    game->item_info[ITEM_CLUB_1].armor_class_bonus = 0;
    game->item_info[ITEM_DAGGER].base_type = BASE_ITEM_DAGGER;
    game->item_info[ITEM_DAGGER].name = "Dagger";
    game->item_info[ITEM_DAGGER].description = "A generic dagger.";
    game->item_info[ITEM_DAGGER].enhancement_bonus = 0;
    game->item_info[ITEM_DAGGER].armor_class_type = 0;
    game->item_info[ITEM_DAGGER].armor_class_bonus = 0;
    game->item_info[ITEM_DAGGER_1].base_type = BASE_ITEM_DAGGER;
    game->item_info[ITEM_DAGGER_1].name = "Dagger + 1";
    game->item_info[ITEM_DAGGER_1].description = "A special dagger.";
    game->item_info[ITEM_DAGGER_1].enhancement_bonus = 1;
    game->item_info[ITEM_DAGGER_1].armor_class_type = 0;
    game->item_info[ITEM_DAGGER_1].armor_class_bonus = 0;
    game->item_info[ITEM_GREATAXE].base_type = BASE_ITEM_GREATAXE;
    game->item_info[ITEM_GREATAXE].name = "Greataxe";
    game->item_info[ITEM_GREATAXE].description = "A generic greataxe.";
    game->item_info[ITEM_GREATAXE].enhancement_bonus = 0;
    game->item_info[ITEM_GREATAXE].armor_class_type = 0;
    game->item_info[ITEM_GREATAXE].armor_class_bonus = 0;
    game->item_info[ITEM_GREATAXE_1].base_type = BASE_ITEM_GREATAXE;
    game->item_info[ITEM_GREATAXE_1].name = "Greataxe + 1";
    game->item_info[ITEM_GREATAXE_1].description = "A special greataxe.";
    game->item_info[ITEM_GREATAXE_1].enhancement_bonus = 1;
    game->item_info[ITEM_GREATAXE_1].armor_class_type = 0;
    game->item_info[ITEM_GREATAXE_1].armor_class_bonus = 0;
    game->item_info[ITEM_HALBERD].base_type = BASE_ITEM_HALBERD;
    game->item_info[ITEM_HALBERD].name = "Halberd";
    game->item_info[ITEM_HALBERD].description = "A generic halberd.";
    game->item_info[ITEM_HALBERD].enhancement_bonus = 0;
    game->item_info[ITEM_HALBERD].armor_class_type = 0;
    game->item_info[ITEM_HALBERD].armor_class_bonus = 0;
    game->item_info[ITEM_HALBERD_1].base_type = BASE_ITEM_HALBERD;
    game->item_info[ITEM_HALBERD_1].name = "Halberd + 1";
    game->item_info[ITEM_HALBERD_1].description = "A special halberd.";
    game->item_info[ITEM_HALBERD_1].enhancement_bonus = 1;
    game->item_info[ITEM_HALBERD_1].armor_class_type = 0;
    game->item_info[ITEM_HALBERD_1].armor_class_bonus = 0;
    game->item_info[ITEM_HEAVY_CROSSBOW].base_type = BASE_ITEM_HEAVY_CROSSBOW;
    game->item_info[ITEM_HEAVY_CROSSBOW].name = "Heavy Crossbow";
    game->item_info[ITEM_HEAVY_CROSSBOW].description = "A generic heavy crossbow.";
    game->item_info[ITEM_HEAVY_CROSSBOW].enhancement_bonus = 0;
    game->item_info[ITEM_HEAVY_CROSSBOW].armor_class_type = 0;
    game->item_info[ITEM_HEAVY_CROSSBOW].armor_class_bonus = 0;
    game->item_info[ITEM_HEAVY_CROSSBOW_1].base_type = BASE_ITEM_HEAVY_CROSSBOW;
    game->item_info[ITEM_HEAVY_CROSSBOW_1].name = "Heavy Crossbow + 1";
    game->item_info[ITEM_HEAVY_CROSSBOW_1].description = "A special heavy crossbow.";
    game->item_info[ITEM_HEAVY_CROSSBOW_1].enhancement_bonus = 1;
    game->item_info[ITEM_HEAVY_CROSSBOW_1].armor_class_type = 0;
    game->item_info[ITEM_HEAVY_CROSSBOW_1].armor_class_bonus = 0;
    game->item_info[ITEM_KATANA].base_type = BASE_ITEM_KATANA;
    game->item_info[ITEM_KATANA].name = "Katana";
    game->item_info[ITEM_KATANA].description = "A generic katana.";
    game->item_info[ITEM_KATANA].enhancement_bonus = 0;
    game->item_info[ITEM_KATANA].armor_class_type = 0;
    game->item_info[ITEM_KATANA].armor_class_bonus = 0;
    game->item_info[ITEM_KATANA_1].base_type = BASE_ITEM_KATANA;
    game->item_info[ITEM_KATANA_1].name = "Katana + 1";
    game->item_info[ITEM_KATANA_1].description = "A special katana.";
    game->item_info[ITEM_KATANA_1].enhancement_bonus = 1;
    game->item_info[ITEM_KATANA_1].armor_class_type = 0;
    game->item_info[ITEM_KATANA_1].armor_class_bonus = 0;
    game->item_info[ITEM_LARGE_SHIELD].base_type = BASE_ITEM_LARGE_SHIELD;
    game->item_info[ITEM_LARGE_SHIELD].name = "Large Shield";
    game->item_info[ITEM_LARGE_SHIELD].description = "A generic large shield.";
    game->item_info[ITEM_LARGE_SHIELD].enhancement_bonus = 0;
    game->item_info[ITEM_LARGE_SHIELD].armor_class_type = 0;
    game->item_info[ITEM_LARGE_SHIELD].armor_class_bonus = 0;
    game->item_info[ITEM_LARGE_SHIELD_1].base_type = BASE_ITEM_LARGE_SHIELD;
    game->item_info[ITEM_LARGE_SHIELD_1].name = "Large Shield + 1";
    game->item_info[ITEM_LARGE_SHIELD_1].description = "A special large shield.";
    game->item_info[ITEM_LARGE_SHIELD_1].enhancement_bonus = 0;
    game->item_info[ITEM_LARGE_SHIELD_1].armor_class_type = ARMOR_CLASS_SHIELD;
    game->item_info[ITEM_LARGE_SHIELD_1].armor_class_bonus = 1;
    game->item_info[ITEM_LIGHT_CROSSBOW].base_type = BASE_ITEM_LIGHT_CROSSBOW;
    game->item_info[ITEM_LIGHT_CROSSBOW].name = "Light Crossbow";
    game->item_info[ITEM_LIGHT_CROSSBOW].description = "A generic light crossbow.";
    game->item_info[ITEM_LIGHT_CROSSBOW].enhancement_bonus = 0;
    game->item_info[ITEM_LIGHT_CROSSBOW].armor_class_type = 0;
    game->item_info[ITEM_LIGHT_CROSSBOW].armor_class_bonus = 0;
    game->item_info[ITEM_LIGHT_CROSSBOW_1].base_type = BASE_ITEM_LIGHT_CROSSBOW;
    game->item_info[ITEM_LIGHT_CROSSBOW_1].name = "Light Crossbow + 1";
    game->item_info[ITEM_LIGHT_CROSSBOW_1].description = "A special light crossbow.";
    game->item_info[ITEM_LIGHT_CROSSBOW_1].enhancement_bonus = 1;
    game->item_info[ITEM_LIGHT_CROSSBOW_1].armor_class_type = 0;
    game->item_info[ITEM_LIGHT_CROSSBOW_1].armor_class_bonus = 0;
    game->item_info[ITEM_LONGBOW].base_type = BASE_ITEM_LONGBOW;
    game->item_info[ITEM_LONGBOW].name = "Longbow";
    game->item_info[ITEM_LONGBOW].description = "A generic longbow.";
    game->item_info[ITEM_LONGBOW].enhancement_bonus = 0;
    game->item_info[ITEM_LONGBOW].armor_class_type = 0;
    game->item_info[ITEM_LONGBOW].armor_class_bonus = 0;
    game->item_info[ITEM_LONGBOW_1].base_type = BASE_ITEM_LONGBOW;
    game->item_info[ITEM_LONGBOW_1].name = "Longbow + 1";
    game->item_info[ITEM_LONGBOW_1].description = "A special longbow.";
    game->item_info[ITEM_LONGBOW_1].enhancement_bonus = 1;
    game->item_info[ITEM_LONGBOW_1].armor_class_type = 0;
    game->item_info[ITEM_LONGBOW_1].armor_class_bonus = 0;
    game->item_info[ITEM_LONGSWORD].base_type = BASE_ITEM_LONGSWORD;
    game->item_info[ITEM_LONGSWORD].name = "Longsword";
    game->item_info[ITEM_LONGSWORD].description = "A generic longsword.";
    game->item_info[ITEM_LONGSWORD].enhancement_bonus = 0;
    game->item_info[ITEM_LONGSWORD].armor_class_type = 0;
    game->item_info[ITEM_LONGSWORD].armor_class_bonus = 0;
    game->item_info[ITEM_LONGSWORD_1].base_type = BASE_ITEM_LONGSWORD;
    game->item_info[ITEM_LONGSWORD_1].name = "Longsword + 1";
    game->item_info[ITEM_LONGSWORD_1].description = "A special longsword.";
    game->item_info[ITEM_LONGSWORD_1].enhancement_bonus = 1;
    game->item_info[ITEM_LONGSWORD_1].armor_class_type = 0;
    game->item_info[ITEM_LONGSWORD_1].armor_class_bonus = 0;
    game->item_info[ITEM_MACE].base_type = BASE_ITEM_MACE;
    game->item_info[ITEM_MACE].name = "Mace";
    game->item_info[ITEM_MACE].description = "A generic mace.";
    game->item_info[ITEM_MACE].enhancement_bonus = 0;
    game->item_info[ITEM_MACE].armor_class_type = 0;
    game->item_info[ITEM_MACE].armor_class_bonus = 0;
    game->item_info[ITEM_MACE_1].base_type = BASE_ITEM_MACE;
    game->item_info[ITEM_MACE_1].name = "Mace + 1";
    game->item_info[ITEM_MACE_1].description = "A special mace.";
    game->item_info[ITEM_MACE_1].enhancement_bonus = 1;
    game->item_info[ITEM_MACE_1].armor_class_type = 0;
    game->item_info[ITEM_MACE_1].armor_class_bonus = 0;
    game->item_info[ITEM_QUARTERSTAFF].base_type = BASE_ITEM_QUARTERSTAFF;
    game->item_info[ITEM_QUARTERSTAFF].name = "Quarterstaff";
    game->item_info[ITEM_QUARTERSTAFF].description = "A generic quarterstaff.";
    game->item_info[ITEM_QUARTERSTAFF].enhancement_bonus = 0;
    game->item_info[ITEM_QUARTERSTAFF].armor_class_type = 0;
    game->item_info[ITEM_QUARTERSTAFF].armor_class_bonus = 0;
    game->item_info[ITEM_QUARTERSTAFF_1].base_type = BASE_ITEM_QUARTERSTAFF;
    game->item_info[ITEM_QUARTERSTAFF_1].name = "Quarterstaff + 1";
    game->item_info[ITEM_QUARTERSTAFF_1].description = "A special quarterstaff.";
    game->item_info[ITEM_QUARTERSTAFF_1].enhancement_bonus = 1;
    game->item_info[ITEM_QUARTERSTAFF_1].armor_class_type = 0;
    game->item_info[ITEM_QUARTERSTAFF_1].armor_class_bonus = 0;
    game->item_info[ITEM_SCIMITAR].base_type = BASE_ITEM_SCIMITAR;
    game->item_info[ITEM_SCIMITAR].name = "Scimitar";
    game->item_info[ITEM_SCIMITAR].description = "A generic scimitar.";
    game->item_info[ITEM_SCIMITAR].enhancement_bonus = 0;
    game->item_info[ITEM_SCIMITAR].armor_class_type = 0;
    game->item_info[ITEM_SCIMITAR].armor_class_bonus = 0;
    game->item_info[ITEM_SCIMITAR_1].base_type = BASE_ITEM_SCIMITAR;
    game->item_info[ITEM_SCIMITAR_1].name = "Scimitar + 1";
    game->item_info[ITEM_SCIMITAR_1].description = "A special scimitar.";
    game->item_info[ITEM_SCIMITAR_1].enhancement_bonus = 1;
    game->item_info[ITEM_SCIMITAR_1].armor_class_type = 0;
    game->item_info[ITEM_SCIMITAR_1].armor_class_bonus = 0;
    game->item_info[ITEM_SHORTBOW].base_type = BASE_ITEM_SHORTBOW;
    game->item_info[ITEM_SHORTBOW].name = "Shortbow";
    game->item_info[ITEM_SHORTBOW].description = "A generic shortbow.";
    game->item_info[ITEM_SHORTBOW].enhancement_bonus = 0;
    game->item_info[ITEM_SHORTBOW].armor_class_type = 0;
    game->item_info[ITEM_SHORTBOW].armor_class_bonus = 0;
    game->item_info[ITEM_SHORTBOW_1].base_type = BASE_ITEM_SHORTBOW;
    game->item_info[ITEM_SHORTBOW_1].name = "Shortbow + 1";
    game->item_info[ITEM_SHORTBOW_1].description = "A special shortbow.";
    game->item_info[ITEM_SHORTBOW_1].enhancement_bonus = 1;
    game->item_info[ITEM_SHORTBOW_1].armor_class_type = 0;
    game->item_info[ITEM_SHORTBOW_1].armor_class_bonus = 0;
    game->item_info[ITEM_SHORTSWORD].base_type = BASE_ITEM_SHORTSWORD;
    game->item_info[ITEM_SHORTSWORD].name = "Shortsword";
    game->item_info[ITEM_SHORTSWORD].description = "A generic shortsword.";
    game->item_info[ITEM_SHORTSWORD].enhancement_bonus = 0;
    game->item_info[ITEM_SHORTSWORD].armor_class_type = 0;
    game->item_info[ITEM_SHORTSWORD].armor_class_bonus = 0;
    game->item_info[ITEM_SHORTSWORD_1].base_type = BASE_ITEM_SHORTSWORD;
    game->item_info[ITEM_SHORTSWORD_1].name = "Shortsword + 1";
    game->item_info[ITEM_SHORTSWORD_1].description = "A special shortsword.";
    game->item_info[ITEM_SHORTSWORD_1].enhancement_bonus = 1;
    game->item_info[ITEM_SHORTSWORD_1].armor_class_type = 0;
    game->item_info[ITEM_SHORTSWORD_1].armor_class_bonus = 0;
    game->item_info[ITEM_SMALL_SHIELD].base_type = BASE_ITEM_SMALL_SHIELD;
    game->item_info[ITEM_SMALL_SHIELD].name = "Small Shield";
    game->item_info[ITEM_SMALL_SHIELD].description = "A generic small shield.";
    game->item_info[ITEM_SMALL_SHIELD].enhancement_bonus = 0;
    game->item_info[ITEM_SMALL_SHIELD].armor_class_type = 0;
    game->item_info[ITEM_SMALL_SHIELD].armor_class_bonus = 0;
    game->item_info[ITEM_SMALL_SHIELD_1].base_type = BASE_ITEM_SMALL_SHIELD;
    game->item_info[ITEM_SMALL_SHIELD_1].name = "Small Shield + 1";
    game->item_info[ITEM_SMALL_SHIELD_1].description = "A special small shield.";
    game->item_info[ITEM_SMALL_SHIELD_1].enhancement_bonus = 0;
    game->item_info[ITEM_SMALL_SHIELD_1].armor_class_type = ARMOR_CLASS_SHIELD;
    game->item_info[ITEM_SMALL_SHIELD_1].armor_class_bonus = 1;
    game->item_info[ITEM_SPEAR].base_type = BASE_ITEM_SPEAR;
    game->item_info[ITEM_SPEAR].name = "Spear";
    game->item_info[ITEM_SPEAR].description = "A generic spear.";
    game->item_info[ITEM_SPEAR].enhancement_bonus = 0;
    game->item_info[ITEM_SPEAR].armor_class_type = 0;
    game->item_info[ITEM_SPEAR].armor_class_bonus = 0;
    game->item_info[ITEM_SPEAR_1].base_type = BASE_ITEM_SPEAR;
    game->item_info[ITEM_SPEAR_1].name = "Spear + 1";
    game->item_info[ITEM_SPEAR_1].description = "A special spear.";
    game->item_info[ITEM_SPEAR_1].enhancement_bonus = 1;
    game->item_info[ITEM_SPEAR_1].armor_class_type = 0;
    game->item_info[ITEM_SPEAR_1].armor_class_bonus = 0;
    game->item_info[ITEM_TOWER_SHIELD].base_type = BASE_ITEM_TOWER_SHIELD;
    game->item_info[ITEM_TOWER_SHIELD].name = "Tower Shield";
    game->item_info[ITEM_TOWER_SHIELD].description = "A generic tower shield.";
    game->item_info[ITEM_TOWER_SHIELD].enhancement_bonus = 0;
    game->item_info[ITEM_TOWER_SHIELD].armor_class_type = 0;
    game->item_info[ITEM_TOWER_SHIELD].armor_class_bonus = 0;
    game->item_info[ITEM_TOWER_SHIELD_1].base_type = BASE_ITEM_TOWER_SHIELD;
    game->item_info[ITEM_TOWER_SHIELD_1].name = "Tower Shield + 1";
    game->item_info[ITEM_TOWER_SHIELD_1].description = "A special tower shield.";
    game->item_info[ITEM_TOWER_SHIELD_1].enhancement_bonus = 0;
    game->item_info[ITEM_TOWER_SHIELD_1].armor_class_type = ARMOR_CLASS_SHIELD;
    game->item_info[ITEM_TOWER_SHIELD_1].armor_class_bonus = 1;
    game->item_info[ITEM_WARHAMMER].base_type = BASE_ITEM_WARHAMMER;
    game->item_info[ITEM_WARHAMMER].name = "Warhammer";
    game->item_info[ITEM_WARHAMMER].description = "A generic warhammer.";
    game->item_info[ITEM_WARHAMMER].enhancement_bonus = 0;
    game->item_info[ITEM_WARHAMMER].armor_class_type = 0;
    game->item_info[ITEM_WARHAMMER].armor_class_bonus = 0;
    game->item_info[ITEM_WARHAMMER_1].base_type = BASE_ITEM_WARHAMMER;
    game->item_info[ITEM_WARHAMMER_1].name = "Warhammer + 1";
    game->item_info[ITEM_WARHAMMER_1].description = "A special warhammer.";
    game->item_info[ITEM_WARHAMMER_1].enhancement_bonus = 1;
    game->item_info[ITEM_WARHAMMER_1].armor_class_type = 0;
    game->item_info[ITEM_WARHAMMER_1].armor_class_bonus = 0;

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
