#include "light.h"

#include "color.h"

const struct light_data light_database[] = {
    [LIGHT_TYPE_NONE] = {
        .radius = -1,
        .color = {COLOR_BLACK},
        .intensity = 0,
        .flicker = false,
    },
    [LIGHT_TYPE_ACID] = {
        .radius = 2,
        .color = {COLOR_LIME},
        .intensity = 0.1f,
        .flicker = true,
    },
    [LIGHT_TYPE_ACID_SPLASH] = {
        .radius = 3,
        .color = {COLOR_LIME},
        .intensity = 0.5f,
        .flicker = true,
    },
    [LIGHT_TYPE_ALTAR] = {
        .radius = 3,
        .color = {COLOR_WHITE},
        .intensity = 0.1f,
        .flicker = false,
    },
    [LIGHT_TYPE_BRAZIER] = {
        .radius = 10,
        .color = {COLOR_LIGHT_AMBER},
        .intensity = 0.25f,
        .flicker = true,
    },
    [LIGHT_TYPE_FIRE] = {
        .radius = 2,
        .color = {COLOR_FLAME},
        .intensity = 0.1f,
        .flicker = true,
    },
    [LIGHT_TYPE_FIREBALL] = {
        .radius = 5,
        .color = {COLOR_FLAME},
        .intensity = 0.5f,
        .flicker = true,
    },
    [LIGHT_TYPE_GLOW] = {
        .radius = 5,
        .color = {COLOR_WHITE},
        .intensity = 0.1f,
        .flicker = false,
    },
    [LIGHT_TYPE_STAIRS] = {
        .radius = 2,
        .color = {COLOR_WHITE},
        .intensity = 0.1f,
        .flicker = false,
    },
    [LIGHT_TYPE_TORCH] = {
        .radius = 10,
        .color = {COLOR_LIGHT_AMBER},
        .intensity = 0.25f,
        .flicker = true,
    },
};
