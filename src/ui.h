#pragma once

#include <stdbool.h>

// Fills the screen with spaces, draws a box border, and writes
// "[ title ]" bold and centred in the top border row.
// Call this instead of clear() + box() at the start of each screen's render.
void ui_screen_frame(const char *title);

// Writes dimmed text left-aligned in the bottom border row (max_y - 1, col 2).
// Use for universal navigation hints such as "[ESC] back  [Enter] confirm".
void ui_footer_hint(const char *text);

// Writes text centred on row y using the supplied curses attribute mask.
// Pass 0 for no attributes.
void ui_centered(int y, const char *text, int attrs);

// Writes text at (y, x). When selected, highlights with A_REVERSE | A_BOLD.
void ui_list_item(int y, int x, const char *text, bool selected);
