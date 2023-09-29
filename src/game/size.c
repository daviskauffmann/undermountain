#include "size.h"

const struct size_data size_database[] = {
    [SIZE_FINE] = {
        .name = "Fine",

        .modifier = 8,
        .speed = 0.1f,
    },
    [SIZE_DIMINUTIVE] = {
        .name = "Diminutive",

        .modifier = 4,
        .speed = 0.25f,
    },
    [SIZE_TINY] = {
        .name = "Tiny",

        .modifier = 2,
        .speed = 0.5f,
    },
    [SIZE_SMALL] = {
        .name = "Small",

        .modifier = 1,
        .speed = 0.75f,
    },
    [SIZE_MEDIUM] = {
        .name = "Medium",

        .modifier = 0,
        .speed = 1,
    },
    [SIZE_LARGE] = {
        .name = "Large",

        .modifier = -1,
        .speed = 0.75f,
    },
    [SIZE_HUGE] = {
        .name = "Huge",

        .modifier = -2,
        .speed = 0.5f,
    },
    [SIZE_GARGANTUAN] = {
        .name = "Gargantuan",

        .modifier = -4,
        .speed = 0.25f,
    },
    [SIZE_COLOSSAL] = {
        .name = "Colossal",

        .modifier = -8,
        .speed = 0.1f,
    },
};
