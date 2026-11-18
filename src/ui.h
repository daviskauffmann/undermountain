#ifndef UM_UI_H
#define UM_UI_H

#include <stdbool.h>

int ui_read_key(void);
bool ui_require_size(int width, int height);

void ui_screen_frame(const char *title);
void ui_footer_hint(const char *text);
void ui_centered(int y, const char *text, int attrs);
void ui_panel_frame(const char *title, const char *hint);
void ui_panel_region_frame(const char *title, const char *hint, int x, int y, int width, int height);

int ui_panel_row(int row, int scroll);
bool ui_panel_row_in_view(int row, int scroll, int max_rows);
void ui_panel_blank(int *row);
void ui_panel_header(int *row, int scroll, int col, int width, const char *text, int max_rows);
void ui_panel_value(int *row, int scroll, int col, int width, const char *label, const char *value, int max_rows);
void ui_panel_item(int *row, int scroll, int col, int width, const char *text, bool selected, int max_rows);
void ui_panel_paragraph(int *row, int scroll, int col, int width, const char *text, int max_rows);
int ui_panel_scroll(int scroll, int total_rows, int max_rows);
int ui_panel_show_row(int scroll, int row, int max_rows);
void ui_panel_divider(int column, int max_rows);
void ui_panel_scrollbar(int total_rows, int max_rows, int max_cols, int scroll);

void ui_list_item(int y, int x, const char *text, bool selected);

#endif
