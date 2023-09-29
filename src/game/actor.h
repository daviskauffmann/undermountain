#ifndef UM_GAME_ACTOR_H
#define UM_GAME_ACTOR_H

#include "ability.h"
#include "base_attack_bonus_progression.h"
#include "equip_slot.h"
#include "faction.h"
#include "item.h"
#include "light.h"
#include "list.h"
#include "natural_weapon.h"
#include "size.h"
#include "special_ability.h"
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
    RACE_ANIMAL,
    RACE_GIANT,
    RACE_HUMANOID,
    RACE_MAGICAL_BEAST,
    RACE_RED_DRAGON_WYRMLING,
    RACE_RED_DRAGON_ADULT,
    RACE_RED_DRAGON_ANCIENT,
    RACE_UNDEAD,
    RACE_VERMIN,

    NUM_RACES,
};
#define PLAYER_RACE_BEGIN RACE_HUMAN
#define PLAYER_RACE_END RACE_ELF
#define NUM_PLAYER_RACES PLAYER_RACE_END + 1
#define MONSTER_RACE_BEGIN RACE_BUGBEAR
#define MONSTER_RACE_END RACE_SLIME
#define NUM_MONSTER_RACES = MONSTER_RACE_END + 1

enum class
{
    // player classes
    CLASS_FIGHTER,
    CLASS_ROGUE,
    CLASS_WIZARD,

    // monster classes
    CLASS_BAT,
    CLASS_BUGBEAR,
    CLASS_DIRE_RAT,
    CLASS_DOG,
    CLASS_FIRE_BEETLE,
    CLASS_HYENA,
    CLASS_GIANT_ANT,
    CLASS_GNOLL,
    CLASS_GOBLIN,
    CLASS_HOBGOBLIN,
    CLASS_KOBOLD,
    CLASS_KRENSHAR,
    CLASS_RAT,
    CLASS_RED_DRAGON_WYRMLING,
    CLASS_RED_DRAGON_ADULT,
    CLASS_RED_DRAGON_ANCIENT,
    CLASS_SKELETON_WARRIOR,
    CLASS_SNAKE,
    CLASS_SPIDER,
    CLASS_TROLL,
    CLASS_WOLF,

    NUM_CLASSES,
};
#define PLAYER_CLASS_BEGIN CLASS_FIGHTER
#define PLAYER_CLASS_END CLASS_WIZARD
#define NUM_PLAYER_CLASSES PLAYER_CLASS_END + 1
#define MONSTER_CLASS_BEGIN CLASS_DOG
#define MONSTER_CLASS_END CLASS_SLIME
#define NUM_MONSTER_CLASSES MONSTER_CLASS_END + 1

enum feat
{
    FEAT_ARMOR_PROFICIENCY_LIGHT,
    FEAT_ARMOR_PROFICIENCY_MEDIUM,
    FEAT_ARMOR_PROFICIENCY_HEAVY,
    FEAT_POINT_BLANK_SHOT,
    FEAT_QUICK_TO_MASTER,
    FEAT_RAPID_RELOAD,
    FEAT_SHIELD_PROFICIENCY,
    FEAT_SNEAK_ATTACK,
    FEAT_STILL_SPELL,
    FEAT_WEAPON_FINESSE,
    FEAT_WEAPON_PROFICIENCY_ELF,
    FEAT_WEAPON_PROFICIENCY_EXOTIC,
    FEAT_WEAPON_PROFICIENCY_MARTIAL,
    FEAT_WEAPON_PROFICIENCY_ROGUE,
    FEAT_WEAPON_PROFICIENCY_SIMPLE,
    FEAT_WEAPON_PROFICIENCY_WIZARD,

    NUM_FEATS,
};

struct race_data
{
    const char *name;

    enum size size;

    bool special_abilities[NUM_SPECIAL_ABILITIES];

    bool feats[NUM_FEATS];
};

struct class_data
{
    const char *name;
    TCOD_ColorRGB color;
    unsigned char glyph;

    const char *hit_die;

    int natural_armor_bonus;

    enum base_attack_bonus_progression base_attack_bonus_progression;
    int base_attack_bonus;

    int default_ability_scores[NUM_ABILITIES];

    int feat_progression[NUM_FEATS];

    int spell_progression[NUM_SPELL_TYPES];

    enum item_type starting_equipment[NUM_EQUIP_SLOTS];

    int starting_items[NUM_ITEM_TYPES];

    enum natural_weapon_type natural_weapon_type;
};

struct feat_prerequisites
{
    bool requires_race;
    enum race race;

    bool requires_class;
    enum class class;

    int level;

    int base_attack_bonus;
};

struct feat_data
{
    const char *name;
    const char *description;

    struct feat_prerequisites prerequisites;
};

struct actor_metadata
{
    int turns_to_chase;
};

struct actor
{
    char *name;

    enum race race;
    enum class class;
    enum size size;
    enum faction faction;

    int level;
    int experience;

    int ability_points;
    int ability_scores[NUM_ABILITIES];

    bool special_abilities[NUM_SPECIAL_ABILITIES];

    bool feats[NUM_FEATS];

    int base_hit_points;
    int hit_points;

    int gold;

    struct item *equipment[NUM_EQUIP_SLOTS];

    struct list *items;

    int mana;
    struct list *known_spells;
    enum spell_type readied_spell;

    int floor;
    int x;
    int y;

    TCOD_Map *fov;

    bool took_turn;
    float energy;

    struct actor *current_target;
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

extern const struct race_data race_database[NUM_RACES];
extern const struct feat_data feat_database[NUM_FEATS];
extern const struct class_data class_database[NUM_CLASSES];
extern const struct actor_metadata actor_metadata;

struct actor *actor_new(
    const char *name,
    enum race race,
    enum class class,
    enum size size,
    enum faction faction,
    int level,
    const int ability_scores[NUM_ABILITIES],
    const bool special_abilities[NUM_SPECIAL_ABILITIES],
    const bool feats[NUM_FEATS],
    int floor,
    int x,
    int y);
void actor_delete(struct actor *actor);

int actor_calc_experience_for_level(int level);
int actor_calc_ability_modifer(const struct actor *actor, enum ability ability);
void actor_give_experience(struct actor *actor, int experience);
void actor_level_up(struct actor *actor);
void actor_add_ability_point(struct actor *actor, enum ability ability);

int actor_calc_max_hit_points(const struct actor *actor);
void actor_restore_hit_points(struct actor *actor, int health);
bool actor_damage_hit_points(struct actor *actor, struct actor *attacker, int damage);

int actor_calc_armor_class(const struct actor *actor);

int actor_calc_attacks_per_round(const struct actor *actor);
int actor_calc_base_attack_bonus(const struct actor *actor);
int actor_calc_attack_bonus(const struct actor *actor);
int actor_calc_ranged_attack_penalty(const struct actor *actor, const struct actor *other);
int actor_calc_threat_range(const struct actor *actor);
int actor_calc_critical_multiplier(const struct actor *actor);
int actor_calc_damage_bonus(const struct actor *actor);
const char *actor_calc_damage(const struct actor *actor);

int actor_calc_max_mana(const struct actor *actor);
void actor_restore_mana(struct actor *actor, int mana);
float actor_calc_arcane_spell_failure(const struct actor *actor);

enum equippability actor_calc_item_equippability(const struct actor *actor, const struct item *item);

float actor_calc_max_carry_weight(const struct actor *actor);
float actor_calc_carry_weight(const struct actor *actor);
float actor_calc_speed(const struct actor *actor);

void actor_calc_light(struct actor *actor);
void actor_calc_fade(struct actor *actor, float delta_time);

int actor_calc_sight_radius(const struct actor *actor);
void actor_calc_fov(struct actor *actor);

void actor_calc_feats(const struct actor *actor, bool (*feats)[NUM_FEATS]);
bool actor_has_feat(const struct actor *actor, enum feat feat);
bool actor_has_prerequisites_for_feat(const struct actor *actor, enum feat feat);

void actor_calc_special_abilities(const struct actor *actor, bool (*special_abilities)[NUM_SPECIAL_ABILITIES]);
bool actor_has_special_ability(const struct actor *actor, enum special_ability special_ability);

void actor_calc_known_spells(const struct actor *actor, bool (*known_spells)[NUM_SPELL_TYPES]);
bool actor_knows_spell(const struct actor *actor, enum spell_type spell_type);

bool actor_can_take_turn(const struct actor *actor);

bool actor_is_enemy(const struct actor *actor, const struct actor *other);
bool actor_is_enemy_nearby(const struct actor *actor);
struct actor *actor_find_nearest_enemy(const struct actor *actor);

bool actor_has_ranged_weapon(const struct actor *actor);
bool actor_is_proficient(const struct actor *actor, const struct item *item);

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

void actor_die(struct actor *actor, struct actor *killer);

#endif
