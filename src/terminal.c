#include "terminal.h"

#include <curses.h>
#include <limits.h>
#include <stdbool.h>
#include <stdio.h>

#define FOREGROUND_COLORS 31
#define BACKGROUND_COLORS 8
#define BASIC_COLORS 8
#define EXTENDED_COLOR_START 16

static const struct color preferred_foreground[FOREGROUND_COLORS] = {
    {0, 0, 0},
    {45, 45, 45},
    {95, 95, 95},
    {127, 127, 127},
    {159, 159, 159},
    {223, 223, 223},
    {255, 255, 255},
    {191, 0, 0},
    {255, 0, 0},
    {255, 63, 63},
    {255, 63, 111},
    {191, 47, 0},
    {255, 63, 0},
    {255, 159, 63},
    {127, 101, 63},
    {158, 134, 100},
    {191, 151, 96},
    {191, 143, 0},
    {255, 191, 0},
    {255, 239, 191},
    {255, 255, 0},
    {0, 191, 0},
    {0, 255, 0},
    {63, 255, 63},
    {191, 255, 0},
    {0, 95, 191},
    {0, 127, 255},
    {63, 159, 255},
    {0, 255, 255},
    {127, 0, 255},
    {255, 191, 223},
};

static const struct color preferred_background[BACKGROUND_COLORS] = {
    {0, 0, 0},
    {32, 32, 32},
    {64, 64, 64},
    {85, 20, 5},
    {75, 45, 12},
    {75, 60, 10},
    {45, 80, 10},
    {10, 45, 95},
};

static const struct color basic_palette[BASIC_COLORS] = {
    {0, 0, 0},
    {0, 0, 170},
    {0, 170, 0},
    {0, 170, 170},
    {170, 0, 0},
    {170, 0, 170},
    {170, 170, 0},
    {200, 200, 200},
};

static struct color foreground_palette[FOREGROUND_COLORS];
static struct color background_palette[BACKGROUND_COLORS];
static int foreground_count;
static int background_count;

static bool initialize_colors(const int first, const struct color *const colors, const int count)
{
    for (int i = 0; i < count; i++)
    {
        const struct color color = colors[i];
        if (init_color((short)(first + i), (short)(color.r * 1000 / 255),
                       (short)(color.g * 1000 / 255), (short)(color.b * 1000 / 255)) == ERR)
        {
            return false;
        }
    }
    return true;
}

static void initialize_terminal_colors(void)
{
    foreground_count = 0;
    background_count = 0;
    if (!has_colors())
    {
        return;
    }
    if (start_color() == ERR)
    {
        fprintf(stderr, "Could not initialize terminal colors; using monochrome.\n");
        return;
    }
    if (COLORS < BASIC_COLORS || COLOR_PAIRS <= BASIC_COLORS * BASIC_COLORS)
    {
        return;
    }

    int first_foreground = 0;
    int first_background = 0;
    if (COLORS >= EXTENDED_COLOR_START + FOREGROUND_COLORS + BACKGROUND_COLORS &&
        COLOR_PAIRS > FOREGROUND_COLORS * BACKGROUND_COLORS && can_change_color())
    {
        first_foreground = EXTENDED_COLOR_START;
        first_background = first_foreground + FOREGROUND_COLORS;
        if (initialize_colors(first_foreground, preferred_foreground, FOREGROUND_COLORS) &&
            initialize_colors(first_background, preferred_background, BACKGROUND_COLORS))
        {
            foreground_count = FOREGROUND_COLORS;
            background_count = BACKGROUND_COLORS;
            for (int i = 0; i < foreground_count; i++)
            {
                foreground_palette[i] = preferred_foreground[i];
            }
            for (int i = 0; i < background_count; i++)
            {
                background_palette[i] = preferred_background[i];
            }
        }
        else
        {
            fprintf(stderr, "Could not initialize the extended terminal palette; using basic colors.\n");
        }
    }

    if (!foreground_count)
    {
        first_foreground = 0;
        first_background = 0;
        foreground_count = BASIC_COLORS;
        background_count = BASIC_COLORS;
        for (int i = 0; i < BASIC_COLORS; i++)
        {
            short r, g, b;
            struct color color = basic_palette[i];
            if (color_content((short)i, &r, &g, &b) != ERR && r >= 0 && g >= 0 && b >= 0)
            {
                color = (struct color){(uint8_t)(r * 255 / 1000),
                                       (uint8_t)(g * 255 / 1000),
                                       (uint8_t)(b * 255 / 1000)};
            }
            foreground_palette[i] = color;
            background_palette[i] = color;
        }
    }

    for (int bg = 0; bg < background_count; bg++)
    {
        for (int fg = 0; fg < foreground_count; fg++)
        {
            const short pair = (short)(1 + bg * foreground_count + fg);
            if (init_pair(pair, (short)(first_foreground + fg), (short)(first_background + bg)) == ERR)
            {
                fprintf(stderr, "Could not initialize terminal color pairs; using monochrome.\n");
                foreground_count = 0;
                background_count = 0;
                return;
            }
        }
    }
}

static int nearest_color(const struct color color, const struct color *const palette, const int count)
{
    int best = 0;
    long best_distance = LONG_MAX;
    for (int i = 0; i < count; i++)
    {
        const long r = (long)color.r - palette[i].r;
        const long g = (long)color.g - palette[i].g;
        const long b = (long)color.b - palette[i].b;
        const long distance = 3 * r * r + 6 * g * g + b * b;
        if (distance < best_distance)
        {
            best_distance = distance;
            best = i;
        }
    }
    return best;
}

bool terminal_init(void)
{
    if (!initscr())
    {
        return false;
    }

    cbreak();
    noecho();
    keypad(stdscr, TRUE);
    nodelay(stdscr, TRUE);
    curs_set(0);

    initialize_terminal_colors();
    return true;
}

void terminal_shutdown(void)
{
    endwin();
}

void terminal_set_color(const struct color foreground, const struct color background, const int attributes)
{
    if (foreground_count)
    {
        const int pair = 1 + nearest_color(background, background_palette, background_count) * foreground_count +
                         nearest_color(foreground, foreground_palette, foreground_count);
        attrset(COLOR_PAIR(pair) | (attributes & ~A_BOLD));
    }
    else
    {
        attrset(attributes);
    }
}

void terminal_reset_color(void)
{
    attrset(A_NORMAL);
}
