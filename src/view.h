#ifndef VIEW_H
#define VIEW_H

#define CONSTRAIN_VIEW true

typedef struct
{
    int left;
    int top;
    int right;
    int bottom;
} view_t;

extern int view_left;
extern int view_top;
extern int view_right;
extern int view_bottom;

void view_update(void);
void view_render(void);

#endif