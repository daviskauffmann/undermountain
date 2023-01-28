#include "color.h"

/* color values */
#define COLOR_BLACK 0, 0, 0
#define COLOR_DARKEST_GREY 31, 31, 31
#define COLOR_DARKER_GREY 63, 63, 63
#define COLOR_DARK_GREY 95, 95, 95
#define COLOR_GREY 127, 127, 127
#define COLOR_LIGHT_GREY 159, 159, 159
#define COLOR_LIGHTER_GREY 191, 191, 191
#define COLOR_LIGHTEST_GREY 223, 223, 223
#define COLOR_WHITE 255, 255, 255

#define COLOR_DARKEST_SEPIA 31, 24, 15
#define COLOR_DARKER_SEPIA 63, 50, 31
#define COLOR_DARK_SEPIA 94, 75, 47
#define COLOR_SEPIA 127, 101, 63
#define COLOR_LIGHT_SEPIA 158, 134, 100
#define COLOR_LIGHTER_SEPIA 191, 171, 143
#define COLOR_LIGHTEST_SEPIA 222, 211, 195

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

/* metallic */
#define COLOR_BRASS 191, 151, 96
#define COLOR_COPPER 197, 136, 124
#define COLOR_GOLD 229, 191, 0
#define COLOR_SILVER 203, 203, 203

/* miscellaneous */
#define COLOR_CELADON 172, 255, 175
#define COLOR_PEACH 255, 159, 127

/* grey levels */
const TCOD_ColorRGB color_black = {COLOR_BLACK};
const TCOD_ColorRGB color_darkest_grey = {COLOR_DARKEST_GREY};
const TCOD_ColorRGB color_darker_grey = {COLOR_DARKER_GREY};
const TCOD_ColorRGB color_dark_grey = {COLOR_DARK_GREY};
const TCOD_ColorRGB color_grey = {COLOR_GREY};
const TCOD_ColorRGB color_light_grey = {COLOR_LIGHT_GREY};
const TCOD_ColorRGB color_lighter_grey = {COLOR_LIGHTER_GREY};
const TCOD_ColorRGB color_lightest_grey = {COLOR_LIGHTEST_GREY};
const TCOD_ColorRGB color_darkest_gray = {COLOR_DARKEST_GREY};
const TCOD_ColorRGB color_darker_gray = {COLOR_DARKER_GREY};
const TCOD_ColorRGB color_dark_gray = {COLOR_DARK_GREY};
const TCOD_ColorRGB color_gray = {COLOR_GREY};
const TCOD_ColorRGB color_light_gray = {COLOR_LIGHT_GREY};
const TCOD_ColorRGB color_lighter_gray = {COLOR_LIGHTER_GREY};
const TCOD_ColorRGB color_lightest_gray = {COLOR_LIGHTEST_GREY};
const TCOD_ColorRGB color_white = {COLOR_WHITE};

/* sepia */
const TCOD_ColorRGB color_darkest_sepia = {COLOR_DARKEST_SEPIA};
const TCOD_ColorRGB color_darker_sepia = {COLOR_DARKER_SEPIA};
const TCOD_ColorRGB color_dark_sepia = {COLOR_DARK_SEPIA};
const TCOD_ColorRGB color_sepia = {COLOR_SEPIA};
const TCOD_ColorRGB color_light_sepia = {COLOR_LIGHT_SEPIA};
const TCOD_ColorRGB color_lighter_sepia = {COLOR_LIGHTER_SEPIA};
const TCOD_ColorRGB color_lightest_sepia = {COLOR_LIGHTEST_SEPIA};

/* standard colors */
const TCOD_ColorRGB color_red = {COLOR_RED};
const TCOD_ColorRGB color_flame = {COLOR_FLAME};
const TCOD_ColorRGB color_orange = {COLOR_ORANGE};
const TCOD_ColorRGB color_amber = {COLOR_AMBER};
const TCOD_ColorRGB color_yellow = {COLOR_YELLOW};
const TCOD_ColorRGB color_lime = {COLOR_LIME};
const TCOD_ColorRGB color_chartreuse = {COLOR_CHARTREUSE};
const TCOD_ColorRGB color_green = {COLOR_GREEN};
const TCOD_ColorRGB color_sea = {COLOR_SEA};
const TCOD_ColorRGB color_turquoise = {COLOR_TURQUOISE};
const TCOD_ColorRGB color_cyan = {COLOR_CYAN};
const TCOD_ColorRGB color_sky = {COLOR_SKY};
const TCOD_ColorRGB color_azure = {COLOR_AZURE};
const TCOD_ColorRGB color_blue = {COLOR_BLUE};
const TCOD_ColorRGB color_han = {COLOR_HAN};
const TCOD_ColorRGB color_violet = {COLOR_VIOLET};
const TCOD_ColorRGB color_purple = {COLOR_PURPLE};
const TCOD_ColorRGB color_fuchsia = {COLOR_FUCHSIA};
const TCOD_ColorRGB color_magenta = {COLOR_MAGENTA};
const TCOD_ColorRGB color_pink = {COLOR_PINK};
const TCOD_ColorRGB color_crimson = {COLOR_CRIMSON};

/* dark colors */
const TCOD_ColorRGB color_dark_red = {COLOR_DARK_RED};
const TCOD_ColorRGB color_dark_flame = {COLOR_DARK_FLAME};
const TCOD_ColorRGB color_dark_orange = {COLOR_DARK_ORANGE};
const TCOD_ColorRGB color_dark_amber = {COLOR_DARK_AMBER};
const TCOD_ColorRGB color_dark_yellow = {COLOR_DARK_YELLOW};
const TCOD_ColorRGB color_dark_lime = {COLOR_DARK_LIME};
const TCOD_ColorRGB color_dark_chartreuse = {COLOR_DARK_CHARTREUSE};
const TCOD_ColorRGB color_dark_green = {COLOR_DARK_GREEN};
const TCOD_ColorRGB color_dark_sea = {COLOR_DARK_SEA};
const TCOD_ColorRGB color_dark_turquoise = {COLOR_DARK_TURQUOISE};
const TCOD_ColorRGB color_dark_cyan = {COLOR_DARK_CYAN};
const TCOD_ColorRGB color_dark_sky = {COLOR_DARK_SKY};
const TCOD_ColorRGB color_dark_azure = {COLOR_DARK_AZURE};
const TCOD_ColorRGB color_dark_blue = {COLOR_DARK_BLUE};
const TCOD_ColorRGB color_dark_han = {COLOR_DARK_HAN};
const TCOD_ColorRGB color_dark_violet = {COLOR_DARK_VIOLET};
const TCOD_ColorRGB color_dark_purple = {COLOR_DARK_PURPLE};
const TCOD_ColorRGB color_dark_fuchsia = {COLOR_DARK_FUCHSIA};
const TCOD_ColorRGB color_dark_magenta = {COLOR_DARK_MAGENTA};
const TCOD_ColorRGB color_dark_pink = {COLOR_DARK_PINK};
const TCOD_ColorRGB color_dark_crimson = {COLOR_DARK_CRIMSON};

/* darker colors */
const TCOD_ColorRGB color_darker_red = {COLOR_DARKER_RED};
const TCOD_ColorRGB color_darker_flame = {COLOR_DARKER_FLAME};
const TCOD_ColorRGB color_darker_orange = {COLOR_DARKER_ORANGE};
const TCOD_ColorRGB color_darker_amber = {COLOR_DARKER_AMBER};
const TCOD_ColorRGB color_darker_yellow = {COLOR_DARKER_YELLOW};
const TCOD_ColorRGB color_darker_lime = {COLOR_DARKER_LIME};
const TCOD_ColorRGB color_darker_chartreuse = {COLOR_DARKER_CHARTREUSE};
const TCOD_ColorRGB color_darker_green = {COLOR_DARKER_GREEN};
const TCOD_ColorRGB color_darker_sea = {COLOR_DARKER_SEA};
const TCOD_ColorRGB color_darker_turquoise = {COLOR_DARKER_TURQUOISE};
const TCOD_ColorRGB color_darker_cyan = {COLOR_DARKER_CYAN};
const TCOD_ColorRGB color_darker_sky = {COLOR_DARKER_SKY};
const TCOD_ColorRGB color_darker_azure = {COLOR_DARKER_AZURE};
const TCOD_ColorRGB color_darker_blue = {COLOR_DARKER_BLUE};
const TCOD_ColorRGB color_darker_han = {COLOR_DARKER_HAN};
const TCOD_ColorRGB color_darker_violet = {COLOR_DARKER_VIOLET};
const TCOD_ColorRGB color_darker_purple = {COLOR_DARKER_PURPLE};
const TCOD_ColorRGB color_darker_fuchsia = {COLOR_DARKER_FUCHSIA};
const TCOD_ColorRGB color_darker_magenta = {COLOR_DARKER_MAGENTA};
const TCOD_ColorRGB color_darker_pink = {COLOR_DARKER_PINK};
const TCOD_ColorRGB color_darker_crimson = {COLOR_DARKER_CRIMSON};

/* darkest colors */
const TCOD_ColorRGB color_darkest_red = {COLOR_DARKEST_RED};
const TCOD_ColorRGB color_darkest_flame = {COLOR_DARKEST_FLAME};
const TCOD_ColorRGB color_darkest_orange = {COLOR_DARKEST_ORANGE};
const TCOD_ColorRGB color_darkest_amber = {COLOR_DARKEST_AMBER};
const TCOD_ColorRGB color_darkest_yellow = {COLOR_DARKEST_YELLOW};
const TCOD_ColorRGB color_darkest_lime = {COLOR_DARKEST_LIME};
const TCOD_ColorRGB color_darkest_chartreuse = {COLOR_DARKEST_CHARTREUSE};
const TCOD_ColorRGB color_darkest_green = {COLOR_DARKEST_GREEN};
const TCOD_ColorRGB color_darkest_sea = {COLOR_DARKEST_SEA};
const TCOD_ColorRGB color_darkest_turquoise = {COLOR_DARKEST_TURQUOISE};
const TCOD_ColorRGB color_darkest_cyan = {COLOR_DARKEST_CYAN};
const TCOD_ColorRGB color_darkest_sky = {COLOR_DARKEST_SKY};
const TCOD_ColorRGB color_darkest_azure = {COLOR_DARKEST_AZURE};
const TCOD_ColorRGB color_darkest_blue = {COLOR_DARKEST_BLUE};
const TCOD_ColorRGB color_darkest_han = {COLOR_DARKEST_HAN};
const TCOD_ColorRGB color_darkest_violet = {COLOR_DARKEST_VIOLET};
const TCOD_ColorRGB color_darkest_purple = {COLOR_DARKEST_PURPLE};
const TCOD_ColorRGB color_darkest_fuchsia = {COLOR_DARKEST_FUCHSIA};
const TCOD_ColorRGB color_darkest_magenta = {COLOR_DARKEST_MAGENTA};
const TCOD_ColorRGB color_darkest_pink = {COLOR_DARKEST_PINK};
const TCOD_ColorRGB color_darkest_crimson = {COLOR_DARKEST_CRIMSON};

/* light colors */
const TCOD_ColorRGB color_light_red = {COLOR_LIGHT_RED};
const TCOD_ColorRGB color_light_flame = {COLOR_LIGHT_FLAME};
const TCOD_ColorRGB color_light_orange = {COLOR_LIGHT_ORANGE};
const TCOD_ColorRGB color_light_amber = {COLOR_LIGHT_AMBER};
const TCOD_ColorRGB color_light_yellow = {COLOR_LIGHT_YELLOW};
const TCOD_ColorRGB color_light_lime = {COLOR_LIGHT_LIME};
const TCOD_ColorRGB color_light_chartreuse = {COLOR_LIGHT_CHARTREUSE};
const TCOD_ColorRGB color_light_green = {COLOR_LIGHT_GREEN};
const TCOD_ColorRGB color_light_sea = {COLOR_LIGHT_SEA};
const TCOD_ColorRGB color_light_turquoise = {COLOR_LIGHT_TURQUOISE};
const TCOD_ColorRGB color_light_cyan = {COLOR_LIGHT_CYAN};
const TCOD_ColorRGB color_light_sky = {COLOR_LIGHT_SKY};
const TCOD_ColorRGB color_light_azure = {COLOR_LIGHT_AZURE};
const TCOD_ColorRGB color_light_blue = {COLOR_LIGHT_BLUE};
const TCOD_ColorRGB color_light_han = {COLOR_LIGHT_HAN};
const TCOD_ColorRGB color_light_violet = {COLOR_LIGHT_VIOLET};
const TCOD_ColorRGB color_light_purple = {COLOR_LIGHT_PURPLE};
const TCOD_ColorRGB color_light_fuchsia = {COLOR_LIGHT_FUCHSIA};
const TCOD_ColorRGB color_light_magenta = {COLOR_LIGHT_MAGENTA};
const TCOD_ColorRGB color_light_pink = {COLOR_LIGHT_PINK};
const TCOD_ColorRGB color_light_crimson = {COLOR_LIGHT_CRIMSON};

/*lighter colors */
const TCOD_ColorRGB color_lighter_red = {COLOR_LIGHTER_RED};
const TCOD_ColorRGB color_lighter_flame = {COLOR_LIGHTER_FLAME};
const TCOD_ColorRGB color_lighter_orange = {COLOR_LIGHTER_ORANGE};
const TCOD_ColorRGB color_lighter_amber = {COLOR_LIGHTER_AMBER};
const TCOD_ColorRGB color_lighter_yellow = {COLOR_LIGHTER_YELLOW};
const TCOD_ColorRGB color_lighter_lime = {COLOR_LIGHTER_LIME};
const TCOD_ColorRGB color_lighter_chartreuse = {COLOR_LIGHTER_CHARTREUSE};
const TCOD_ColorRGB color_lighter_green = {COLOR_LIGHTER_GREEN};
const TCOD_ColorRGB color_lighter_sea = {COLOR_LIGHTER_SEA};
const TCOD_ColorRGB color_lighter_turquoise = {COLOR_LIGHTER_TURQUOISE};
const TCOD_ColorRGB color_lighter_cyan = {COLOR_LIGHTER_CYAN};
const TCOD_ColorRGB color_lighter_sky = {COLOR_LIGHTER_SKY};
const TCOD_ColorRGB color_lighter_azure = {COLOR_LIGHTER_AZURE};
const TCOD_ColorRGB color_lighter_blue = {COLOR_LIGHTER_BLUE};
const TCOD_ColorRGB color_lighter_han = {COLOR_LIGHTER_HAN};
const TCOD_ColorRGB color_lighter_violet = {COLOR_LIGHTER_VIOLET};
const TCOD_ColorRGB color_lighter_purple = {COLOR_LIGHTER_PURPLE};
const TCOD_ColorRGB color_lighter_fuchsia = {COLOR_LIGHTER_FUCHSIA};
const TCOD_ColorRGB color_lighter_magenta = {COLOR_LIGHTER_MAGENTA};
const TCOD_ColorRGB color_lighter_pink = {COLOR_LIGHTER_PINK};
const TCOD_ColorRGB color_lighter_crimson = {COLOR_LIGHTER_CRIMSON};

/* lightest colors */
const TCOD_ColorRGB color_lightest_red = {COLOR_LIGHTEST_RED};
const TCOD_ColorRGB color_lightest_flame = {COLOR_LIGHTEST_FLAME};
const TCOD_ColorRGB color_lightest_orange = {COLOR_LIGHTEST_ORANGE};
const TCOD_ColorRGB color_lightest_amber = {COLOR_LIGHTEST_AMBER};
const TCOD_ColorRGB color_lightest_yellow = {COLOR_LIGHTEST_YELLOW};
const TCOD_ColorRGB color_lightest_lime = {COLOR_LIGHTEST_LIME};
const TCOD_ColorRGB color_lightest_chartreuse = {COLOR_LIGHTEST_CHARTREUSE};
const TCOD_ColorRGB color_lightest_green = {COLOR_LIGHTEST_GREEN};
const TCOD_ColorRGB color_lightest_sea = {COLOR_LIGHTEST_SEA};
const TCOD_ColorRGB color_lightest_turquoise = {COLOR_LIGHTEST_TURQUOISE};
const TCOD_ColorRGB color_lightest_cyan = {COLOR_LIGHTEST_CYAN};
const TCOD_ColorRGB color_lightest_sky = {COLOR_LIGHTEST_SKY};
const TCOD_ColorRGB color_lightest_azure = {COLOR_LIGHTEST_AZURE};
const TCOD_ColorRGB color_lightest_blue = {COLOR_LIGHTEST_BLUE};
const TCOD_ColorRGB color_lightest_han = {COLOR_LIGHTEST_HAN};
const TCOD_ColorRGB color_lightest_violet = {COLOR_LIGHTEST_VIOLET};
const TCOD_ColorRGB color_lightest_purple = {COLOR_LIGHTEST_PURPLE};
const TCOD_ColorRGB color_lightest_fuchsia = {COLOR_LIGHTEST_FUCHSIA};
const TCOD_ColorRGB color_lightest_magenta = {COLOR_LIGHTEST_MAGENTA};
const TCOD_ColorRGB color_lightest_pink = {COLOR_LIGHTEST_PINK};
const TCOD_ColorRGB color_lightest_crimson = {COLOR_LIGHTEST_CRIMSON};

/* desaturated colors */
const TCOD_ColorRGB color_desaturated_red = {COLOR_DESATURATED_RED};
const TCOD_ColorRGB color_desaturated_flame = {COLOR_DESATURATED_FLAME};
const TCOD_ColorRGB color_desaturated_orange = {COLOR_DESATURATED_ORANGE};
const TCOD_ColorRGB color_desaturated_amber = {COLOR_DESATURATED_AMBER};
const TCOD_ColorRGB color_desaturated_yellow = {COLOR_DESATURATED_YELLOW};
const TCOD_ColorRGB color_desaturated_lime = {COLOR_DESATURATED_LIME};
const TCOD_ColorRGB color_desaturated_chartreuse = {COLOR_DESATURATED_CHARTREUSE};
const TCOD_ColorRGB color_desaturated_green = {COLOR_DESATURATED_GREEN};
const TCOD_ColorRGB color_desaturated_sea = {COLOR_DESATURATED_SEA};
const TCOD_ColorRGB color_desaturated_turquoise = {COLOR_DESATURATED_TURQUOISE};
const TCOD_ColorRGB color_desaturated_cyan = {COLOR_DESATURATED_CYAN};
const TCOD_ColorRGB color_desaturated_sky = {COLOR_DESATURATED_SKY};
const TCOD_ColorRGB color_desaturated_azure = {COLOR_DESATURATED_AZURE};
const TCOD_ColorRGB color_desaturated_blue = {COLOR_DESATURATED_BLUE};
const TCOD_ColorRGB color_desaturated_han = {COLOR_DESATURATED_HAN};
const TCOD_ColorRGB color_desaturated_violet = {COLOR_DESATURATED_VIOLET};
const TCOD_ColorRGB color_desaturated_purple = {COLOR_DESATURATED_PURPLE};
const TCOD_ColorRGB color_desaturated_fuchsia = {COLOR_DESATURATED_FUCHSIA};
const TCOD_ColorRGB color_desaturated_magenta = {COLOR_DESATURATED_MAGENTA};
const TCOD_ColorRGB color_desaturated_pink = {COLOR_DESATURATED_PINK};
const TCOD_ColorRGB color_desaturated_crimson = {COLOR_DESATURATED_CRIMSON};

/* metallic */
const TCOD_ColorRGB color_brass = {COLOR_BRASS};
const TCOD_ColorRGB color_copper = {COLOR_COPPER};
const TCOD_ColorRGB color_gold = {COLOR_GOLD};
const TCOD_ColorRGB color_silver = {COLOR_SILVER};

/* miscellaneous */
const TCOD_ColorRGB color_celadon = {COLOR_CELADON};
const TCOD_ColorRGB color_peach = {COLOR_PEACH};
