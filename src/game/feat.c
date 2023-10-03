#include "feat.h"

#include "class.h"
#include "race.h"

const struct feat_data feat_database[] = {
    [FEAT_ALERTNESS] = {
        .name = "Alertness",
        .description = "A character with this feat gets a +2 bonus on all spot and listen checks.",
    },
    [FEAT_ARMOR_PROFICIENCY_LIGHT] = {
        .name = "Armor Proficiency: Light",
        .description = "A character with this feat can equip light armor.",
    },
    [FEAT_ARMOR_PROFICIENCY_MEDIUM] = {
        .name = "Armor Proficiency: Medium",
        .description = "A character with this feat can equip medium armor.",
    },
    [FEAT_ARMOR_PROFICIENCY_HEAVY] = {
        .name = "Armor Proficiency: Heavy",
        .description = "A character with this feat can equip heavy armor.",
    },
    [FEAT_COMBAT_CASTING] = {
        .name = "Combat Casting",
        .description = "A character with this feat gets a +4 bonus on concentration checks made to cast a spell or use a spell-like ability when on the defensive or while grappling or pinned.",
    },
    [FEAT_DODGE] = {
        .name = "Dodge",
        .description = "A character with this feat gets a +1 dodge bonus to armor class.",

        .prerequisites = {
            .ability_scores = {
                [ABILITY_DEXTERITY] = 13,
            },
        },
    },
    [FEAT_IMPROVED_INITIATIVE] = {
        .name = "Improved Initiative",
        .description = "A character with this feat gets a +4 bonus on initiative checks.",
    },
    [FEAT_IRON_WILL] = {
        .name = "Iron Will",
        .description = "A character with this feat gets a +2 bonus on all will saving throws.",
    },
    [FEAT_MULTIATTACK] = {
        .name = "Multiattack",
        .description = "A creature with this feat is particularly skilled at making multiple attacks.",

        .prerequisites = {
            .base_attack_bonus = 6,
        },
    },
    [FEAT_POINT_BLANK_SHOT] = {
        .name = "Point Blank Shot",
        .description = "A character with this feat does not suffer a -4 penalty on ranged attack rolls when firing into melee.",
    },
    [FEAT_POWER_ATTACK] = {
        .name = "Power Attack",
        .description = "A character with this feat can make exceptionally powerful melee attacks.",

        .prerequisites = {
            .ability_scores = {
                [ABILITY_STRENGTH] = 13,
            },
        },
    },
    [FEAT_QUICK_TO_MASTER] = {
        .name = "Quick to Master",
        .description = "A character with this feat gains an additional feat at level 1.",

        .prerequisites = {
            .race = RACE_HUMAN,
        },
    },
    [FEAT_RAPID_RELOAD] = {
        .name = "Rapid Reload",
        .description = "A character gets the same number of attacks with any crossbow as a normal bow.",

        .prerequisites = {
            .base_attack_bonus = 2,
        },
    },
    [FEAT_SHIELD_PROFICIENCY] = {
        .name = "Shield Proficiency",
        .description = "A character with this feat can equip shields.",
    },
    [FEAT_SNEAK_ATTACK] = {
        .name = "Sneak Attack",
        .description = "A character with this feat deals extra damage when undetected.",

        .prerequisites = {
            .class = CLASS_ROGUE,
        },
    },
    [FEAT_STEALTHY] = {
        .name = "Stealthy",
        .description = "A character with this feat gets a +2 bonus on all hide and move silently checks.",
    },
    [FEAT_STILL_SPELL] = {
        .name = "Still Spell",
        .description = "A character with this feat ignores arcane spell failure, but all arcane spell mana costs are increased by 1.",
    },
    [FEAT_TOUGHNESS] = {
        .name = "Toughness",
        .description = "A character with this feat retroactively gains 1 hit point per level.",
    },
    [FEAT_TRACK] = {
        .name = "Track",
        .description = "A character with this feat can track creatures by following their tracks.",
    },
    [FEAT_WEAPON_FINESSE] = {
        .name = "Weapon Finesse",
        .description = "A character with this feat is adept at using light weapons subtly and effectively, allowing him to make melee attack rolls with his dexterity modifier instead of strength (if his dexterity is higher than his strength).",

        .prerequisites = {
            .base_attack_bonus = 1,
        },
    },
    [FEAT_WEAPON_FOCUS] = {
        .name = "Weapon Focus",
        .description = "A character with this feat gains a +1 bonus on all attack rolls he makes using the selected weapon.",

        .prerequisites = {
            .base_attack_bonus = 1,
        },
    },
    [FEAT_WEAPON_PROFICIENCY_ELF] = {
        .name = "Weapon Proficiency: Elf",
        .description = "A character with this feat is proficient with elf weapons.",

        .prerequisites = {
            .race = RACE_ELF,
        },
    },
    [FEAT_WEAPON_PROFICIENCY_EXOTIC] = {
        .name = "Weapon Proficiency: Exotic",
        .description = "A character with this feat is proficient with exotic weapons.",
    },
    [FEAT_WEAPON_PROFICIENCY_MARTIAL] = {
        .name = "Weapon Proficiency: Martial",
        .description = "A character with this feat is proficient with martial weapons.",
    },
    [FEAT_WEAPON_PROFICIENCY_ROGUE] = {
        .name = "Weapon Proficiency: Rogue",
        .description = "A character with this feat is proficient with rogue weapons.",

        .prerequisites = {
            .class = CLASS_ROGUE,
        },
    },
    [FEAT_WEAPON_PROFICIENCY_SIMPLE] = {
        .name = "Weapon Proficiency: Simple",
        .description = "A character with this feat is proficient with simple weapons.",
    },
    [FEAT_WEAPON_PROFICIENCY_WIZARD] = {
        .name = "Weapon Proficiency: Wizard",
        .description = "A character with this feat is proficient with wizard weapons.",

        .prerequisites = {
            .class = CLASS_WIZARD,
        },
    },
};
