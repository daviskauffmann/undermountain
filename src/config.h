#ifndef CONFIG_H
#define CONFIG_H

#include <libtcod.h>

#define TITLE "Undermountain"
#define VERSION "0.0.1"
#define SAVE_PATH "data/save.gz"

extern int console_width;
extern int console_height;

void config_load(void);

#endif
