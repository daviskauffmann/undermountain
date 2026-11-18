#include "ui.h"

#include <curses.h>
#include <stdio.h>
#include <string.h>

static int panel_origin_x;
static int panel_origin_y;

int ui_read_key(void)
{
    const int key = getch();
    if (key != KEY_RESIZE)
    {
        return key;
    }

    if (resize_term(0, 0) == ERR)
    {
        fprintf(stderr, "Could not resize the terminal screen.\n");
    }
    clearok(stdscr, TRUE);
    return ERR;
}

bool ui_require_size(const int width, const int height)
{
    if (getmaxx(stdscr) >= width && getmaxy(stdscr) >= height)
    {
        return true;
    }

    erase();
    ui_centered(0, "Enlarge terminal", A_BOLD);
    refresh();
    return false;
}

void ui_screen_frame(const char *const title)
{
    int max_y, max_x;
    getmaxyx(stdscr, max_y, max_x);

    for (int y = 0; y < max_y; y++)
    {
        mvhline(y, 0, ' ', max_x);
    }

    box(stdscr, 0, 0);

    char buffer[128];
    snprintf(buffer, sizeof(buffer), "[ %s ]", title);
    const int length = (int)strlen(buffer);
    int col = (max_x - length) / 2;
    if (col < 1)
    {
        col = 1;
    }
    attron(A_BOLD);
    if (max_x > col + 1)
    {
        mvaddnstr(0, col, buffer, max_x - col - 1);
    }
    attroff(A_BOLD);
}

void ui_footer_hint(const char *const text)
{
    const int max_y = getmaxy(stdscr);
    const int width = getmaxx(stdscr) - 4;
    attron(A_DIM);
    if (max_y > 0 && width > 0)
    {
        mvaddnstr(max_y - 1, 2, text, width);
    }
    attroff(A_DIM);
}

void ui_centered(const int y, const char *const text, const int attrs)
{
    const int max_x = getmaxx(stdscr);
    int col = (max_x - (int)strlen(text)) / 2;
    if (col < 0)
    {
        col = 0;
    }
    if (attrs)
    {
        attron(attrs);
    }
    if (y >= 0 && y < getmaxy(stdscr) && max_x > col)
    {
        mvaddnstr(y, col, text, max_x - col);
    }
    if (attrs)
    {
        attroff(attrs);
    }
}

void ui_panel_frame(const char *const title, const char *const hint)
{
    panel_origin_x = 0;
    panel_origin_y = 0;
    ui_screen_frame(title);
    ui_footer_hint(hint);
}

void ui_panel_region_frame(const char *const title, const char *const hint,
                           const int x, const int y, const int width, const int height)
{
    panel_origin_x = x;
    panel_origin_y = y;

    for (int row = y; row < y + height; row++)
    {
        mvhline(row, x, ' ', width);
    }
    if (width < 2 || height < 2)
    {
        return;
    }

    mvhline(y, x, ACS_HLINE, width);
    mvhline(y + height - 1, x, ACS_HLINE, width);
    mvvline(y, x, ACS_VLINE, height);
    mvvline(y, x + width - 1, ACS_VLINE, height);
    mvaddch(y, x, ACS_ULCORNER);
    mvaddch(y, x + width - 1, ACS_URCORNER);
    mvaddch(y + height - 1, x, ACS_LLCORNER);
    mvaddch(y + height - 1, x + width - 1, ACS_LRCORNER);

    char buffer[128];
    snprintf(buffer, sizeof(buffer), "[ %s ]", title);
    const int length = (int)strlen(buffer);
    int col = x + (width - length) / 2;
    if (col < x + 1)
    {
        col = x + 1;
    }
    attron(A_BOLD);
    if (x + width - 1 > col)
    {
        mvaddnstr(y, col, buffer, x + width - 1 - col);
    }
    attroff(A_BOLD);

    const int hint_width = width - 4;
    if (hint_width > 0)
    {
        attron(A_DIM);
        mvaddnstr(y + height - 1, x + 2, hint, hint_width);
        attroff(A_DIM);
    }
}

int ui_panel_row(const int row, const int scroll)
{
    return panel_origin_y + row - scroll + 1;
}

bool ui_panel_row_in_view(const int row, const int scroll, const int max_rows)
{
    const int local_row = row - scroll + 1;
    return local_row >= 1 && local_row <= max_rows - 2;
}

void ui_panel_blank(int *const row)
{
    (*row)++;
}

void ui_panel_header(int *const row, const int scroll, const int col, const int width, const char *const text, const int max_rows)
{
    if (width > 0 && ui_panel_row_in_view(*row, scroll, max_rows))
    {
        attron(A_BOLD | A_UNDERLINE);
        mvprintw(ui_panel_row(*row, scroll), panel_origin_x + col, "%-.*s", width, text);
        attroff(A_BOLD | A_UNDERLINE);
    }
    (*row)++;
}

void ui_panel_value(int *const row, const int scroll, const int col, const int width, const char *const label, const char *const value, const int max_rows)
{
    const int screen_row = ui_panel_row(*row, scroll);
    if (width > 0 && ui_panel_row_in_view(*row, scroll, max_rows))
    {
        mvprintw(screen_row, panel_origin_x + col, "%-.*s", width, label);
        const int value_len = (int)strlen(value);
        const int value_col = panel_origin_x + col + width - value_len;
        if (value_col >= panel_origin_x + col)
        {
            mvprintw(screen_row, value_col, "%s", value);
        }
    }
    (*row)++;
}

void ui_panel_item(int *const row, const int scroll, const int col, const int width, const char *const text, const bool selected, const int max_rows)
{
    if (width > 0 && ui_panel_row_in_view(*row, scroll, max_rows))
    {
        if (selected)
        {
            attron(A_REVERSE | A_BOLD);
        }
        mvaddnstr(ui_panel_row(*row, scroll), panel_origin_x + col, text, width);
        if (selected)
        {
            attroff(A_REVERSE | A_BOLD);
        }
    }
    (*row)++;
}

void ui_panel_paragraph(int *const row, const int scroll, const int col, const int width, const char *text, const int max_rows)
{
    if (width <= 0)
    {
        return;
    }

    while (*text)
    {
        size_t length = 0;
        while (text[length] && text[length] != '\n' && length < (size_t)width)
        {
            length++;
        }
        if (length == (size_t)width && text[length] && text[length] != '\n')
        {
            size_t word_end = length;
            while (word_end > 0 && text[word_end] != ' ')
            {
                word_end--;
            }
            if (word_end > 0)
            {
                length = word_end;
            }
        }
        if (ui_panel_row_in_view(*row, scroll, max_rows))
        {
            mvaddnstr(ui_panel_row(*row, scroll), panel_origin_x + col, text, (int)length);
        }
        (*row)++;
        text += length;
        if (*text == '\n')
        {
            text++;
        }
        else
        {
            while (*text == ' ')
            {
                text++;
            }
        }
    }
}

int ui_panel_scroll(int scroll, const int total_rows, const int max_rows)
{
    const int visible_rows = max_rows > 2 ? max_rows - 2 : 1;
    const int limit = total_rows > visible_rows ? total_rows - visible_rows : 0;
    if (scroll > limit)
    {
        scroll = limit;
    }
    return scroll < 0 ? 0 : scroll;
}

int ui_panel_show_row(const int scroll, const int row, const int max_rows)
{
    const int visible_rows = max_rows > 2 ? max_rows - 2 : 1;
    if (row < scroll)
    {
        return row;
    }
    if (row >= scroll + visible_rows)
    {
        return row - visible_rows + 1;
    }
    return scroll;
}

void ui_panel_divider(const int column, const int max_rows)
{
    for (int y = 1; y < max_rows - 1; y++)
    {
        mvaddch(panel_origin_y + y, panel_origin_x + column, ACS_VLINE);
    }
}

void ui_panel_scrollbar(const int total_rows, const int max_rows, const int max_cols, int scroll)
{
    scroll = ui_panel_scroll(scroll, total_rows, max_rows);
    if (scroll > 0)
    {
        mvaddch(panel_origin_y + 1, panel_origin_x + max_cols - 1, ACS_UARROW);
    }
    if ((total_rows - scroll) > max_rows - 2)
    {
        mvaddch(panel_origin_y + max_rows - 2, panel_origin_x + max_cols - 1, ACS_DARROW);
    }
}

void ui_list_item(const int y, const int x, const char *const text, const bool selected)
{
    if (selected)
    {
        attron(A_REVERSE | A_BOLD);
    }
    mvprintw(y, x, "%s", text);
    if (selected)
    {
        attroff(A_REVERSE | A_BOLD);
    }
}
