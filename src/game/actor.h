#ifndef UM_GAME_ACTOR_H
#define UM_GAME_ACTOR_H

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
    RACE_HUMAN,
    RACE_DWARF,
    RACE_ELF,

    // monster races
    RACE_BUGBEAR,
    RACE_DRAGON,
    RACE_DOG,
    RACE_GOBLIN,
    RACE_JACKAL,
    RACE_KOBOLD,
    RACE_ORC,
    RACE_RAT,
    RACE_SLIME,

    NUM_RACES
};
#define PLAYER_RACE_BEGIN RACE_HUMAN
#define PLAYER_RACE_END RACE_ELF
#define MONSTER_RACE_BEGIN RACE_BUGBEAR
#define MONSTER_RACE_END RACE_SLIME

enum class
{
    // player classes
    CLASS_FIGHTER,
    CLASS_ROGUE,
    CLASS_WIZARD,

    // monster classes
    CLASS_DOG,
    CLASS_DRAGON,
    CLASS_GOBLIN,
    CLASS_JACKAL,
    CLASS_KOBOLD,
    CLASS_RAT,
    CLASS_SLIME,

    NUM_CLASSES
};
#define PLAYER_CLASS_BEGIN CLASS_FIGHTER
#define PLAYER_CLASS_END CLASS_WIZARD
#define MONSTER_CLASS_BEGIN CLASS_DOG
#define MONSTER_CLASS_END CLASS_SLIME

enum base_attack_bonus_progression
{
    BASE_ATTACK_BONUS_PROGRESSION_COMBAT,
    BASE_ATTACK_BONUS_PROGRESSION_MIDDLE,
    BASE_ATTACK_BONUS_PROGRESSION_NON_COMBAT,

    NUM_BASE_ATTACK_BONUS_PROGRESSIONS
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

enum feat
{
    FEAT_LOW_LIGHT_VISION,
    FEAT_WEAPON_FINESSE,

    NUM_FEATS
};

enum monster
{
    MONSTER_BUGBEAR,
    MONSTER_GOBLIN,
    MONSTER_JACKAL,
    MONSTER_KOBOLD,
    MONSTER_ORC,
    MONSTER_RAT,
    MONSTER_RED_DRAGON,
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

struct actor_metadata
{
    int turns_to_chase;
};

struct race_data
{
    const char *name;
    unsigned char glyph;

    enum size size;
    float speed;

    bool feats[NUM_FEATS];
};

struct class_data
{
    const char *name;
    TCOD_ColorRGB color;

    const char *hit_die;
    const char *mana_die;

    enum base_attack_bonus_progression base_attack_bonus_progression;

    int default_ability_scores[NUM_ABILITIES];

    bool feats[NUM_FEATS];

    int spell_progression[NUM_SPELL_TYPES];

    enum item_type starting_equipment[NUM_EQUIP_SLOTS];

    int starting_items[NUM_ITEM_TYPES];
};

struct base_attack_bonus_progression_data
{
    const char *name;
    float multiplier;
};

struct ability_data
{
    const char *name;
    const char *description;
};

struct feat_data
{
    const char *name;
    const char *description;
};

struct actor_prototype
{
    const char *name;

    enum race race;
    enum class class;

    int level;

    int ability_scores[NUM_ABILITIES];

    bool feats[NUM_FEATS];

    enum item_type equipment[NUM_EQUIP_SLOTS];

    // TODO: inventory/spells
};

struct actor
{
    char *name;

    enum race race;
    enum class class;

    int level;
    int experience;

    int ability_points;
    int ability_scores[NUM_ABILITIES];

    bool feats[NUM_FEATS];

    int base_hit_points;
    int base_mana_points;

    int hit_points;
    int mana_points;

    int gold;

    struct item *equipment[NUM_EQUIP_SLOTS];

    struct list *items;

    struct list *known_spells;
    enum spell_type readied_spell;

    enum faction faction;

    int floor;
    int x;
    int y;

    TCOD_Map *fov;

    bool took_turn;
    float energy;

    int last_seen_x;
    int last_seen_y;
    int turns_chased;

    struct actor *leader;

    enum light_type light_type;
    TCOD_Map *light_fov;

    TCOD_ColorRGB flash_color;
    float flash_fade_coef;

    bool controllable;

    bool dead;
};

struct actor *actor_new(
    const char *name,
    enum race race,
    enum class class,
    int level,
    const int ability_scores[NUM_ABILITIES],
    const bool feats[NUM_FEATS],
    enum faction faction,
    int floor,
    int x,
    int y);
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
float actor_calc_max_carry_weight(const struct actor *actor);
float actor_calc_carry_weight(const struct actor *actor);
float actor_calc_speed(const struct actor *actor);

void actor_calc_light(struct actor *actor);
void actor_calc_fade(struct actor *actor, float delta_time);

int actor_calc_sight_radius(const struct actor *actor);
void actor_calc_fov(struct actor *actor);

void actor_give_experience(struct actor *actor, int experience);
void actor_level_up(struct actor *actor);
void actor_add_ability_point(struct actor *actor, enum ability ability);

bool actor_has_feat(const struct actor *actor, enum feat feat);

void actor_calc_known_spells(const struct actor *actor, bool (*known_spells)[NUM_SPELL_TYPES]);
bool actor_knows_spell(const struct actor *actor, enum spell_type spell_type);

bool actor_can_take_turn(const struct actor *actor);

bool actor_is_enemy(const struct actor *actor, const struct actor *other);
bool actor_is_enemy_nearby(const struct actor *actor);
struct actor *actor_find_nearest_enemy(const struct actor *actor);

bool actor_has_ranged_weapon(const struct actor *actor);

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
bool actor_dip(
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
bool actor_cast(
    struct actor *actor,
    enum spell_type spell_type,
    int x, int y,
    bool from_memory);

void actor_restore_hit_points(struct actor *actor, int health);
void actor_restore_mana_points(struct actor *actor, int mana);
bool actor_damage_hit_points(struct actor *actor, struct actor *attacker, int damage);
void actor_die(struct actor *actor, struct actor *killer);

#endif
