#ifndef GAME_color_H
#define GAME_color_H

#include <libtcod.h>

/* grey levels */
#define COLOR_BLACK 0, 0, 0
#define COLOR_DARKEST_GRAY 31, 31, 31
#define COLOR_DARKER_GRAY 63, 63, 63
#define COLOR_DARK_GRAY 95, 95, 95
#define COLOR_GRAY 127, 127, 127
#define COLOR_LIGHT_GRAY 159, 159, 159
#define COLOR_LIGHTER_GRAY 191, 191, 191
#define COLOR_LIGHTEST_GRAY 223, 223, 223
#define COLOR_WHITE 255, 255, 255

/* normal */
#define COLOR_RED 255, 0, 0
#define COLOR_FLAME 255, 63, 0
#define COLOR_ORANGE 255, 127, 0
#define COLOR_AMBER 255, 191, 0
#define COLOR_YELLOW 255, 255, 0
#define COLOR_LIME 191, 255, 0
#define COLOR_CHARTREUSE 127, 255, 0
#define COLOR_GREEN 0, 255, 0
#define COLOR_SEA 0, 255, 127
#define COLOR_TURQUOISE 0, 255, 191
#define COLOR_CYAN 0, 255, 255
#define COLOR_SKY 0, 191, 255
#define COLOR_AZURE 0, 127, 255
#define COLOR_BLUE 0, 0, 255
#define COLOR_HAN 63, 0, 255
#define COLOR_VIOLET 127, 0, 255
#define COLOR_PURPLE 191, 0, 255
#define COLOR_FUCHSIA 255, 0, 255
#define COLOR_MAGENTA 255, 0, 191
#define COLOR_PINK 255, 0, 127
#define COLOR_CRIMSON 255, 0, 63

/* lightest */
#define COLOR_LIGHTEST_RED 255, 191, 191
#define COLOR_LIGHTEST_FLAME 255, 207, 191
#define COLOR_LIGHTEST_ORANGE 255, 223, 191
#define COLOR_LIGHTEST_AMBER 255, 239, 191
#define COLOR_LIGHTEST_YELLOW 255, 255, 191
#define COLOR_LIGHTEST_LIME 239, 255, 191
#define COLOR_LIGHTEST_CHARTREUSE 223, 255, 191
#define COLOR_LIGHTEST_GREEN 191, 255, 191
#define COLOR_LIGHTEST_SEA 191, 255, 223
#define COLOR_LIGHTEST_TURQUOISE 191, 255, 239
#define COLOR_LIGHTEST_CYAN 191, 255, 255
#define COLOR_LIGHTEST_SKY 191, 239, 255
#define COLOR_LIGHTEST_AZURE 191, 223, 255
#define COLOR_LIGHTEST_BLUE 191, 191, 255
#define COLOR_LIGHTEST_HAN 207, 191, 255
#define COLOR_LIGHTEST_VIOLET 223, 191, 255
#define COLOR_LIGHTEST_PURPLE 239, 191, 255
#define COLOR_LIGHTEST_FUCHSIA 255, 191, 255
#define COLOR_LIGHTEST_MAGENTA 255, 191, 239
#define COLOR_LIGHTEST_PINK 255, 191, 223
#define COLOR_LIGHTEST_CRIMSON 255, 191, 207

/* lighter */
#define COLOR_LIGHTER_RED 255, 127, 127
#define COLOR_LIGHTER_FLAME 255, 159, 127
#define COLOR_LIGHTER_ORANGE 255, 191, 127
#define COLOR_LIGHTER_AMBER 255, 223, 127
#define COLOR_LIGHTER_YELLOW 255, 255, 127
#define COLOR_LIGHTER_LIME 223, 255, 127
#define COLOR_LIGHTER_CHARTREUSE 191, 255, 127
#define COLOR_LIGHTER_GREEN 127, 255, 127
#define COLOR_LIGHTER_SEA 127, 255, 191
#define COLOR_LIGHTER_TURQUOISE 127, 255, 223
#define COLOR_LIGHTER_CYAN 127, 255, 255
#define COLOR_LIGHTER_SKY 127, 223, 255
#define COLOR_LIGHTER_AZURE 127, 191, 255
#define COLOR_LIGHTER_BLUE 127, 127, 255
#define COLOR_LIGHTER_HAN 159, 127, 255
#define COLOR_LIGHTER_VIOLET 191, 127, 255
#define COLOR_LIGHTER_PURPLE 223, 127, 255
#define COLOR_LIGHTER_FUCHSIA 255, 127, 255
#define COLOR_LIGHTER_MAGENTA 255, 127, 223
#define COLOR_LIGHTER_PINK 255, 127, 191
#define COLOR_LIGHTER_CRIMSON 255, 127, 159

/* light */
#define COLOR_LIGHT_RED 255, 63, 63
#define COLOR_LIGHT_FLAME 255, 111, 63
#define COLOR_LIGHT_ORANGE 255, 159, 63
#define COLOR_LIGHT_AMBER 255, 207, 63
#define COLOR_LIGHT_YELLOW 255, 255, 63
#define COLOR_LIGHT_LIME 207, 255, 63
#define COLOR_LIGHT_CHARTREUSE 159, 255, 63
#define COLOR_LIGHT_GREEN 63, 255, 63
#define COLOR_LIGHT_SEA 63, 255, 159
#define COLOR_LIGHT_TURQUOISE 63, 255, 207
#define COLOR_LIGHT_CYAN 63, 255, 255
#define COLOR_LIGHT_SKY 63, 207, 255
#define COLOR_LIGHT_AZURE 63, 159, 255
#define COLOR_LIGHT_BLUE 63, 63, 255
#define COLOR_LIGHT_HAN 111, 63, 255
#define COLOR_LIGHT_VIOLET 159, 63, 255
#define COLOR_LIGHT_PURPLE 207, 63, 255
#define COLOR_LIGHT_FUCHSIA 255, 63, 255
#define COLOR_LIGHT_MAGENTA 255, 63, 207
#define COLOR_LIGHT_PINK 255, 63, 159
#define COLOR_LIGHT_CRIMSON 255, 63, 111

/* dark */
#define COLOR_DARK_RED 191, 0, 0
#define COLOR_DARK_FLAME 191, 47, 0
#define COLOR_DARK_ORANGE 191, 95, 0
#define COLOR_DARK_AMBER 191, 143, 0
#define COLOR_DARK_YELLOW 191, 191, 0
#define COLOR_DARK_LIME 143, 191, 0
#define COLOR_DARK_CHARTREUSE 95, 191, 0
#define COLOR_DARK_GREEN 0, 191, 0
#define COLOR_DARK_SEA 0, 191, 95
#define COLOR_DARK_TURQUOISE 0, 191, 143
#define COLOR_DARK_CYAN 0, 191, 191
#define COLOR_DARK_SKY 0, 143, 191
#define COLOR_DARK_AZURE 0, 95, 191
#define COLOR_DARK_BLUE 0, 0, 191
#define COLOR_DARK_HAN 47, 0, 191
#define COLOR_DARK_VIOLET 95, 0, 191
#define COLOR_DARK_PURPLE 143, 0, 191
#define COLOR_DARK_FUCHSIA 191, 0, 191
#define COLOR_DARK_MAGENTA 191, 0, 143
#define COLOR_DARK_PINK 191, 0, 95
#define COLOR_DARK_CRIMSON 191, 0, 47

/* darker */
#define COLOR_DARKER_RED 127, 0, 0
#define COLOR_DARKER_FLAME 127, 31, 0
#define COLOR_DARKER_ORANGE 127, 63, 0
#define COLOR_DARKER_AMBER 127, 95, 0
#define COLOR_DARKER_YELLOW 127, 127, 0
#define COLOR_DARKER_LIME 95, 127, 0
#define COLOR_DARKER_CHARTREUSE 63, 127, 0
#define COLOR_DARKER_GREEN 0, 127, 0
#define COLOR_DARKER_SEA 0, 127, 63
#define COLOR_DARKER_TURQUOISE 0, 127, 95
#define COLOR_DARKER_CYAN 0, 127, 127
#define COLOR_DARKER_SKY 0, 95, 127
#define COLOR_DARKER_AZURE 0, 63, 127
#define COLOR_DARKER_BLUE 0, 0, 127
#define COLOR_DARKER_HAN 31, 0, 127
#define COLOR_DARKER_VIOLET 63, 0, 127
#define COLOR_DARKER_PURPLE 95, 0, 127
#define COLOR_DARKER_FUCHSIA 127, 0, 127
#define COLOR_DARKER_MAGENTA 127, 0, 95
#define COLOR_DARKER_PINK 127, 0, 63
#define COLOR_DARKER_CRIMSON 127, 0, 31

/* darkest */
#define COLOR_DARKEST_RED 63, 0, 0
#define COLOR_DARKEST_FLAME 63, 15, 0
#define COLOR_DARKEST_ORANGE 63, 31, 0
#define COLOR_DARKEST_AMBER 63, 47, 0
#define COLOR_DARKEST_YELLOW 63, 63, 0
#define COLOR_DARKEST_LIME 47, 63, 0
#define COLOR_DARKEST_CHARTREUSE 31, 63, 0
#define COLOR_DARKEST_GREEN 0, 63, 0
#define COLOR_DARKEST_SEA 0, 63, 31
#define COLOR_DARKEST_TURQUOISE 0, 63, 47
#define COLOR_DARKEST_CYAN 0, 63, 63
#define COLOR_DARKEST_SKY 0, 47, 63
#define COLOR_DARKEST_AZURE 0, 31, 63
#define COLOR_DARKEST_BLUE 0, 0, 63
#define COLOR_DARKEST_HAN 15, 0, 63
#define COLOR_DARKEST_VIOLET 31, 0, 63
#define COLOR_DARKEST_PURPLE 47, 0, 63
#define COLOR_DARKEST_FUCHSIA 63, 0, 63
#define COLOR_DARKEST_MAGENTA 63, 0, 47
#define COLOR_DARKEST_PINK 63, 0, 31
#define COLOR_DARKEST_CRIMSON 63, 0, 15

/* desaturated */
#define COLOR_DESATURATED_RED 127, 63, 63
#define COLOR_DESATURATED_FLAME 127, 79, 63
#define COLOR_DESATURATED_ORANGE 127, 95, 63
#define COLOR_DESATURATED_AMBER 127, 111, 63
#define COLOR_DESATURATED_YELLOW 127, 127, 63
#define COLOR_DESATURATED_LIME 111, 127, 63
#define COLOR_DESATURATED_CHARTREUSE 95, 127, 63
#define COLOR_DESATURATED_GREEN 63, 127, 63
#define COLOR_DESATURATED_SEA 63, 127, 95
#define COLOR_DESATURATED_TURQUOISE 63, 127, 111
#define COLOR_DESATURATED_CYAN 63, 127, 127
#define COLOR_DESATURATED_SKY 63, 111, 127
#define COLOR_DESATURATED_AZURE 63, 95, 127
#define COLOR_DESATURATED_BLUE 63, 63, 127
#define COLOR_DESATURATED_HAN 79, 63, 127
#define COLOR_DESATURATED_VIOLET 95, 63, 127
#define COLOR_DESATURATED_PURPLE 111, 63, 127
#define COLOR_DESATURATED_FUCHSIA 127, 63, 127
#define COLOR_DESATURATED_MAGENTA 127, 63, 111
#define COLOR_DESATURATED_PINK 127, 63, 95
#define COLOR_DESATURATED_CRIMSON 127, 63, 79

/* sepia */
#define COLOR_DARKEST_SEPIA 31, 24, 15
#define COLOR_DARKER_SEPIA 63, 50, 31
#define COLOR_DARK_SEPIA 94, 75, 47
#define COLOR_SEPIA 127, 101, 63
#define COLOR_LIGHT_SEPIA 158, 134, 100
#define COLOR_LIGHTER_SEPIA 191, 171, 143
#define COLOR_LIGHTEST_SEPIA 222, 211, 195

/* metallic */
#define COLOR_BRASS 191, 151, 96
#define COLOR_COPPER 197, 136, 124
#define COLOR_GOLD 229, 191, 0
#define COLOR_SILVER 203, 203, 203

/* miscellaneous */
#define COLOR_CELADON 172, 255, 175
#define COLOR_PEACH 255, 159, 127

/* grey levels */
extern const TCOD_ColorRGB color_black;
extern const TCOD_ColorRGB color_darkest_gray;
extern const TCOD_ColorRGB color_darker_gray;
extern const TCOD_ColorRGB color_dark_gray;
extern const TCOD_ColorRGB color_gray;
extern const TCOD_ColorRGB color_light_gray;
extern const TCOD_ColorRGB color_lighter_gray;
extern const TCOD_ColorRGB color_lightest_gray;
extern const TCOD_ColorRGB color_white;

/* standard colors */
extern const TCOD_ColorRGB color_red;
extern const TCOD_ColorRGB color_flame;
extern const TCOD_ColorRGB color_orange;
extern const TCOD_ColorRGB color_amber;
extern const TCOD_ColorRGB color_yellow;
extern const TCOD_ColorRGB color_lime;
extern const TCOD_ColorRGB color_chartreuse;
extern const TCOD_ColorRGB color_green;
extern const TCOD_ColorRGB color_sea;
extern const TCOD_ColorRGB color_turquoise;
extern const TCOD_ColorRGB color_cyan;
extern const TCOD_ColorRGB color_sky;
extern const TCOD_ColorRGB color_azure;
extern const TCOD_ColorRGB color_blue;
extern const TCOD_ColorRGB color_han;
extern const TCOD_ColorRGB color_violet;
extern const TCOD_ColorRGB color_purple;
extern const TCOD_ColorRGB color_fuchsia;
extern const TCOD_ColorRGB color_magenta;
extern const TCOD_ColorRGB color_pink;
extern const TCOD_ColorRGB color_crimson;

/* dark colors */
extern const TCOD_ColorRGB color_dark_red;
extern const TCOD_ColorRGB color_dark_flame;
extern const TCOD_ColorRGB color_dark_orange;
extern const TCOD_ColorRGB color_dark_amber;
extern const TCOD_ColorRGB color_dark_yellow;
extern const TCOD_ColorRGB color_dark_lime;
extern const TCOD_ColorRGB color_dark_chartreuse;
extern const TCOD_ColorRGB color_dark_green;
extern const TCOD_ColorRGB color_dark_sea;
extern const TCOD_ColorRGB color_dark_turquoise;
extern const TCOD_ColorRGB color_dark_cyan;
extern const TCOD_ColorRGB color_dark_sky;
extern const TCOD_ColorRGB color_dark_azure;
extern const TCOD_ColorRGB color_dark_blue;
extern const TCOD_ColorRGB color_dark_han;
extern const TCOD_ColorRGB color_dark_violet;
extern const TCOD_ColorRGB color_dark_purple;
extern const TCOD_ColorRGB color_dark_fuchsia;
extern const TCOD_ColorRGB color_dark_magenta;
extern const TCOD_ColorRGB color_dark_pink;
extern const TCOD_ColorRGB color_dark_crimson;

/* darker colors */
extern const TCOD_ColorRGB color_darker_red;
extern const TCOD_ColorRGB color_darker_flame;
extern const TCOD_ColorRGB color_darker_orange;
extern const TCOD_ColorRGB color_darker_amber;
extern const TCOD_ColorRGB color_darker_yellow;
extern const TCOD_ColorRGB color_darker_lime;
extern const TCOD_ColorRGB color_darker_chartreuse;
extern const TCOD_ColorRGB color_darker_green;
extern const TCOD_ColorRGB color_darker_sea;
extern const TCOD_ColorRGB color_darker_turquoise;
extern const TCOD_ColorRGB color_darker_cyan;
extern const TCOD_ColorRGB color_darker_sky;
extern const TCOD_ColorRGB color_darker_azure;
extern const TCOD_ColorRGB color_darker_blue;
extern const TCOD_ColorRGB color_darker_han;
extern const TCOD_ColorRGB color_darker_violet;
extern const TCOD_ColorRGB color_darker_purple;
extern const TCOD_ColorRGB color_darker_fuchsia;
extern const TCOD_ColorRGB color_darker_magenta;
extern const TCOD_ColorRGB color_darker_pink;
extern const TCOD_ColorRGB color_darker_crimson;

/* darkest colors */
extern const TCOD_ColorRGB color_darkest_red;
extern const TCOD_ColorRGB color_darkest_flame;
extern const TCOD_ColorRGB color_darkest_orange;
extern const TCOD_ColorRGB color_darkest_amber;
extern const TCOD_ColorRGB color_darkest_yellow;
extern const TCOD_ColorRGB color_darkest_lime;
extern const TCOD_ColorRGB color_darkest_chartreuse;
extern const TCOD_ColorRGB color_darkest_green;
extern const TCOD_ColorRGB color_darkest_sea;
extern const TCOD_ColorRGB color_darkest_turquoise;
extern const TCOD_ColorRGB color_darkest_cyan;
extern const TCOD_ColorRGB color_darkest_sky;
extern const TCOD_ColorRGB color_darkest_azure;
extern const TCOD_ColorRGB color_darkest_blue;
extern const TCOD_ColorRGB color_darkest_han;
extern const TCOD_ColorRGB color_darkest_violet;
extern const TCOD_ColorRGB color_darkest_purple;
extern const TCOD_ColorRGB color_darkest_fuchsia;
extern const TCOD_ColorRGB color_darkest_magenta;
extern const TCOD_ColorRGB color_darkest_pink;
extern const TCOD_ColorRGB color_darkest_crimson;

/* light colors */
extern const TCOD_ColorRGB color_light_red;
extern const TCOD_ColorRGB color_light_flame;
extern const TCOD_ColorRGB color_light_orange;
extern const TCOD_ColorRGB color_light_amber;
extern const TCOD_ColorRGB color_light_yellow;
extern const TCOD_ColorRGB color_light_lime;
extern const TCOD_ColorRGB color_light_chartreuse;
extern const TCOD_ColorRGB color_light_green;
extern const TCOD_ColorRGB color_light_sea;
extern const TCOD_ColorRGB color_light_turquoise;
extern const TCOD_ColorRGB color_light_cyan;
extern const TCOD_ColorRGB color_light_sky;
extern const TCOD_ColorRGB color_light_azure;
extern const TCOD_ColorRGB color_light_blue;
extern const TCOD_ColorRGB color_light_han;
extern const TCOD_ColorRGB color_light_violet;
extern const TCOD_ColorRGB color_light_purple;
extern const TCOD_ColorRGB color_light_fuchsia;
extern const TCOD_ColorRGB color_light_magenta;
extern const TCOD_ColorRGB color_light_pink;
extern const TCOD_ColorRGB color_light_crimson;

/*lighter colors */
extern const TCOD_ColorRGB color_lighter_red;
extern const TCOD_ColorRGB color_lighter_flame;
extern const TCOD_ColorRGB color_lighter_orange;
extern const TCOD_ColorRGB color_lighter_amber;
extern const TCOD_ColorRGB color_lighter_yellow;
extern const TCOD_ColorRGB color_lighter_lime;
extern const TCOD_ColorRGB color_lighter_chartreuse;
extern const TCOD_ColorRGB color_lighter_green;
extern const TCOD_ColorRGB color_lighter_sea;
extern const TCOD_ColorRGB color_lighter_turquoise;
extern const TCOD_ColorRGB color_lighter_cyan;
extern const TCOD_ColorRGB color_lighter_sky;
extern const TCOD_ColorRGB color_lighter_azure;
extern const TCOD_ColorRGB color_lighter_blue;
extern const TCOD_ColorRGB color_lighter_han;
extern const TCOD_ColorRGB color_lighter_violet;
extern const TCOD_ColorRGB color_lighter_purple;
extern const TCOD_ColorRGB color_lighter_fuchsia;
extern const TCOD_ColorRGB color_lighter_magenta;
extern const TCOD_ColorRGB color_lighter_pink;
extern const TCOD_ColorRGB color_lighter_crimson;

/* lightest colors */
extern const TCOD_ColorRGB color_lightest_red;
extern const TCOD_ColorRGB color_lightest_flame;
extern const TCOD_ColorRGB color_lightest_orange;
extern const TCOD_ColorRGB color_lightest_amber;
extern const TCOD_ColorRGB color_lightest_yellow;
extern const TCOD_ColorRGB color_lightest_lime;
extern const TCOD_ColorRGB color_lightest_chartreuse;
extern const TCOD_ColorRGB color_lightest_green;
extern const TCOD_ColorRGB color_lightest_sea;
extern const TCOD_ColorRGB color_lightest_turquoise;
extern const TCOD_ColorRGB color_lightest_cyan;
extern const TCOD_ColorRGB color_lightest_sky;
extern const TCOD_ColorRGB color_lightest_azure;
extern const TCOD_ColorRGB color_lightest_blue;
extern const TCOD_ColorRGB color_lightest_han;
extern const TCOD_ColorRGB color_lightest_violet;
extern const TCOD_ColorRGB color_lightest_purple;
extern const TCOD_ColorRGB color_lightest_fuchsia;
extern const TCOD_ColorRGB color_lightest_magenta;
extern const TCOD_ColorRGB color_lightest_pink;
extern const TCOD_ColorRGB color_lightest_crimson;

/* desaturated colors */
extern const TCOD_ColorRGB color_desaturated_red;
extern const TCOD_ColorRGB color_desaturated_flame;
extern const TCOD_ColorRGB color_desaturated_orange;
extern const TCOD_ColorRGB color_desaturated_amber;
extern const TCOD_ColorRGB color_desaturated_yellow;
extern const TCOD_ColorRGB color_desaturated_lime;
extern const TCOD_ColorRGB color_desaturated_chartreuse;
extern const TCOD_ColorRGB color_desaturated_green;
extern const TCOD_ColorRGB color_desaturated_sea;
extern const TCOD_ColorRGB color_desaturated_turquoise;
extern const TCOD_ColorRGB color_desaturated_cyan;
extern const TCOD_ColorRGB color_desaturated_sky;
extern const TCOD_ColorRGB color_desaturated_azure;
extern const TCOD_ColorRGB color_desaturated_blue;
extern const TCOD_ColorRGB color_desaturated_han;
extern const TCOD_ColorRGB color_desaturated_violet;
extern const TCOD_ColorRGB color_desaturated_purple;
extern const TCOD_ColorRGB color_desaturated_fuchsia;
extern const TCOD_ColorRGB color_desaturated_magenta;
extern const TCOD_ColorRGB color_desaturated_pink;
extern const TCOD_ColorRGB color_desaturated_crimson;

/* sepia */
extern const TCOD_ColorRGB color_darkest_sepia;
extern const TCOD_ColorRGB color_darker_sepia;
extern const TCOD_ColorRGB color_dark_sepia;
extern const TCOD_ColorRGB color_sepia;
extern const TCOD_ColorRGB color_light_sepia;
extern const TCOD_ColorRGB color_lighter_sepia;
extern const TCOD_ColorRGB color_lightest_sepia;

/* metallic */
extern const TCOD_ColorRGB color_brass;
extern const TCOD_ColorRGB color_copper;
extern const TCOD_ColorRGB color_gold;
extern const TCOD_ColorRGB color_silver;

/* miscellaneous */
extern const TCOD_ColorRGB color_celadon;
extern const TCOD_ColorRGB color_peach;

#endif
