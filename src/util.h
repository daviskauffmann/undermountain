#ifndef UM_UTIL_H
#define UM_UTIL_H

#include <stdbool.h>

#define PI 3.14f

bool file_exists(const char *filename);
void file_delete(const char *filename);

float distance_between_sq(
    int x1, int y1,
    int x2, int y2);
float distance_between(
    int x1, int y1,
    int x2, int y2);
float angle_between(
    int x1, int y1,
    int x2, int y2);

#endif
