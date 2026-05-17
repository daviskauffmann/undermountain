#include "ui.h"

#include <curses.h>
#include <string.h>
#include <stdio.h>

void ui_screen_frame(const char *title)
{
    int max_y, max_x;
    getmaxyx(stdscr, max_y, max_x);

    // Blank every row so previously drawn content (e.g. map tiles) is erased.
    for (int y = 0; y < max_y; y++)
    {
        mvhline(y, 0, ' ', max_x);
    }

    box(stdscr, 0, 0);

    // Centred bold "[ title ]" embedded in the top border row.
    char buf[128];
    snprintf(buf, sizeof(buf), "[ %s ]", title);
    int len = (int)strlen(buf);
    int col = (max_x - len) / 2;
    if (col < 1)
    {
        col = 1;
    }
    attron(A_BOLD);
    mvprintw(0, col, "%s", buf);
    attroff(A_BOLD);
}

void ui_footer_hint(const char *text)
{
    int max_y = getmaxy(stdscr);
    attron(A_DIM);
    mvprintw(max_y - 1, 2, "%s", text);
    attroff(A_DIM);
}

void ui_centered(int y, const char *text, int attrs)
{
    int max_x = getmaxx(stdscr);
    int col = (max_x - (int)strlen(text)) / 2;
    if (col < 0)
        col = 0;
    if (attrs)
        attron(attrs);
    mvprintw(y, col, "%s", text);
    if (attrs)
        attroff(attrs);
}

void ui_list_item(int y, int x, const char *text, bool selected)
{
    if (selected)
        attron(A_REVERSE | A_BOLD);
    mvprintw(y, x, "%s", text);
    if (selected)
        attroff(A_REVERSE | A_BOLD);
}
