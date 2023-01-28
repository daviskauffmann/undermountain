#ifndef CONFIG_H
#define CONFIG_H

#include <libtcod.h>

#define TITLE "Undermountain"
#define VERSION "0.0.1"
#define SAVE_PATH "data/save"

extern int console_width;
extern int console_height;

extern const char *tileset_filename;
extern int tileset_columns;
extern int tileset_rows;
extern const int *tileset_charmap;

void config_load(void);

#endif
