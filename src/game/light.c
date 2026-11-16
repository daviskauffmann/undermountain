#include "light.h"

const struct light_data light_database[] = {
    [LIGHT_TYPE_NONE] = {
        .radius = -1,
    },
    [LIGHT_TYPE_ACID] = {
        .radius = 1,
    },
    [LIGHT_TYPE_ACID_SPLASH] = {
        .radius = 2,
    },
    [LIGHT_TYPE_ALTAR] = {
        .radius = 3,
    },
    [LIGHT_TYPE_BRAZIER] = {
        .radius = 10,
    },
    [LIGHT_TYPE_FIRE] = {
        .radius = 1,
    },
    [LIGHT_TYPE_FIREBALL] = {
        .radius = 5,
    },
    [LIGHT_TYPE_GLOW] = {
        .radius = 5,
    },
    [LIGHT_TYPE_MAGIC_MISSILE] = {
        .radius = 2,
    },
    [LIGHT_TYPE_PLAYER] = {
        .radius = -1,
    },
    [LIGHT_TYPE_STAIRS] = {
        .radius = 2,
    },
    [LIGHT_TYPE_TORCH] = {
        .radius = 10,
    },
};
