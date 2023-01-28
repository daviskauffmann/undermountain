#ifndef GAME_ACTOR_H
#define GAME_ACTOR_H

#include "item.h"
#include "light.h"
#include "list.h"
#include "size.h"
#include "spell.h"
#include <libtcod.h>

struct object;

enum race
{
    // player races
    RACE_DWARF,
    RACE_ELF,
    RACE_HUMAN,

    // monster races
    RACE_BUGBEAR,
    RACE_DOG,
    RACE_GOBLIN,
    RACE_JACKAL,
    RACE_KOBOLD,
    RACE_ORC,
    RACE_RAT,
    RACE_SLIME,

    NUM_RACES
};

enum class
{
    // player classes
    CLASS_FIGHTER,
    CLASS_ROGUE,
    CLASS_WIZARD,

    // monster classes
    CLASS_DOG,
    CLASS_GOBLIN,
    CLASS_JACKAL,
    CLASS_KOBOLD,
    CLASS_RAT,
    CLASS_SLIME,

    NUM_CLASSES
};

enum faction
{
    FACTION_ADVENTURER,
    FACTION_MONSTER
};

enum ability
{
    ABILITY_STRENGTH,
    ABILITY_DEXTERITY,
    ABILITY_CONSTITUTION,
    ABILITY_INTELLIGENCE,

    NUM_ABILITIES
};

enum monster
{
    MONSTER_BUGBEAR,
    MONSTER_GOBLIN,
    MONSTER_JACKAL,
    MONSTER_KOBOLD,
    MONSTER_ORC,
    MONSTER_RAT,
    MONSTER_SLIME,

    NUM_MONSTERS
};

enum equippability
{
    EQUIPPABILITY_TOO_LARGE,
    EQUIPPABILITY_BARELY,
    EQUIPPABILITY_COMFORTABLY,
    EQUIPPABILITY_EASILY,
    EQUIPPABILITY_TOO_SMALL
};

struct actor_common
{
    int turns_to_chase;
};

struct race_data
{
    const char *name;
    unsigned char glyph;

    enum size size;
    float speed;
};

struct class_data
{
    const char *name;
    TCOD_color_t color;

    const char *hit_die;
    const char *mana_die;

    // TODO: base attack bonus
};

struct ability_data
{
    const char *name;
};

struct actor_prototype
{
    const char *name;
    enum race race;
    enum class class;

    uint8_t level;

    int ability_scores[NUM_ABILITIES];

    enum item_type equipment[NUM_EQUIP_SLOTS];

    // TODO: equipment/inventory/spells
};

struct actor
{
    char *name;
    enum race race;
    enum class class;
    enum faction faction;

    uint8_t level;
    int experience;
    int ability_points;

    int ability_scores[NUM_ABILITIES];

    int base_hit_points;
    int base_mana_points;

    int hit_points;
    int mana_points;

    int gold;
    struct item *equipment[NUM_EQUIP_SLOTS];
    struct list *items;
    struct list *known_spell_types;
    enum spell_type readied_spell_type;

    uint8_t floor;
    uint8_t x;
    uint8_t y;

    TCOD_map_t fov;

    bool took_turn;
    float energy;

    int last_seen_x;
    int last_seen_y;
    int turns_chased;

    struct actor *leader;

    enum light_type light_type;
    TCOD_map_t light_fov;

    TCOD_color_t flash_color;
    float flash_fade_coef;

    bool controllable;

    bool dead;
};

struct actor *actor_new(
    const char *name,
    enum race race,
    enum class class,
    enum faction faction,
    uint8_t floor,
    uint8_t x,
    uint8_t y);
void actor_delete(struct actor *actor);

int actor_calc_experience_for_level(int level);
int actor_calc_ability_modifer(const struct actor *actor, enum ability ability);
int actor_calc_max_hit_points(const struct actor *actor);
int actor_calc_max_mana_points(const struct actor *actor);
int actor_calc_armor_class(const struct actor *actor);
int actor_calc_base_attack_bonus(const struct actor *actor);
int actor_calc_attack_bonus(const struct actor *actor);
int actor_calc_threat_range(const struct actor *actor);
int actor_calc_critical_multiplier(const struct actor *actor);
int actor_calc_damage_bonus(const struct actor *actor);
const char *actor_calc_damage(const struct actor *actor);
enum equippability actor_calc_item_equippability(const struct actor *actor, const struct item *item);
float actor_calc_speed(const struct actor *actor);

void actor_calc_light(struct actor *actor);
void actor_calc_fade(struct actor *actor, float delta_time);

int actor_calc_sight_radius(struct actor *actor);
void actor_calc_fov(struct actor *actor);

void actor_give_experience(struct actor *actor, int experience);
void actor_level_up(struct actor *actor);
void actor_add_ability_point(struct actor *actor, enum ability ability);

bool actor_can_take_turn(const struct actor *actor);

struct actor *actor_find_closest_enemy(const struct actor *actor);

bool actor_ai(struct actor *actor);
bool actor_rest(struct actor *actor);
bool actor_path_towards(
    struct actor *actor,
    int target_x, int target_y);
bool actor_move_towards(
    struct actor *actor,
    int target_x, int target_y);
bool actor_move(
    struct actor *actor,
    int x, int y);
bool actor_swap(struct actor *actor, struct actor *other);
bool actor_interact(struct actor *actor, struct object *object);
bool actor_open_door(
    struct actor *actor,
    int x, int y);
bool actor_close_door(
    struct actor *actor,
    int x, int y);
bool actor_descend(struct actor *actor);
bool actor_ascend(struct actor *actor);
bool actor_open_chest(
    struct actor *actor,
    int x, int y);
bool actor_pray(
    struct actor *actor,
    int x, int y);
bool actor_drink(
    struct actor *actor,
    int x, int y);
bool actor_sit(
    struct actor *actor,
    int x, int y);
bool actor_grab(
    struct actor *actor,
    int x, int y);
bool actor_drop(struct actor *actor, struct item *item);
bool actor_equip(struct actor *actor, struct item *item);
bool actor_unequip(struct actor *actor, enum equip_slot equip_slot);
bool actor_quaff(struct actor *actor, struct item *item);
bool actor_read(
    struct actor *actor,
    struct item *item,
    int x, int y);
bool actor_bash(struct actor *actor, struct object *object);
bool actor_shoot(
    struct actor *actor,
    int x, int y);
bool actor_attack(struct actor *actor, struct actor *other, const struct item *ammunition);
bool actor_cast_spell(
    struct actor *actor,
    enum spell_type spell_type,
    int x, int y,
    bool from_memory);

void actor_restore_hit_points(struct actor *actor, int health);
void actor_restore_mana_points(struct actor *actor, int mana);
bool actor_damage_hit_points(struct actor *actor, struct actor *attacker, int damage);
void actor_die(struct actor *actor, struct actor *killer);

#endif
