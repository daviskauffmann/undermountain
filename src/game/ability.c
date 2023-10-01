#include "ability.h"

const struct ability_data ability_database[] = {
    [ABILITY_STRENGTH] = {
        .name = "Strength",
        .description = "Strength measures the muscle and physical power of a character.",
    },
    [ABILITY_DEXTERITY] = {
        .name = "Dexterity",
        .description = "Dexterity measures agility, reflexes, and balance.",
    },
    [ABILITY_CONSTITUTION] = {
        .name = "Constitution",
        .description = "Constitution represents the health and stamina of a character.",
    },
    [ABILITY_INTELLIGENCE] = {
        .name = "Intelligence",
        .description = "Intelligence determines how well a character learns and reasons.",
    },
    [ABILITY_WISDOM] = {
        .name = "Wisdom",
        .description = "Wisdom describes a character's willpower, common sense, perception, and intuition.",
    },
    [ABILITY_CHARISMA] = {
        .name = "Charisma",
        .description = "Charisma measures a character's force of personality, persuasiveness, ability to lead, and physical attractiveness.",
    },
};
