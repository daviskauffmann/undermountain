#ifndef PLATFORM_H
#define PLATFORM_H

#include <libtcod/libtcod.h>
#include <roguelike/roguelike.h>

#include "config.h"
#include "input.h"
#include "renderer.h"
#include "tooltip_option.h"
#include "ui.h"

int platform_run(void);
bool file_exists(const char *filename);

#endif
