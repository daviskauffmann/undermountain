#ifndef UTILS_H
#define UTILS_H

#include <stdint.h>

#define global static
#define internal static
#define local static

typedef int8_t i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

extern double
distance_sq(int x1, int y1, int x2, int y2);

extern double
distance(int x1, int y1, int x2, int y2);

extern double
angle(int x1, int y1, int x2, int y2);

extern int
roll(int a, int x);

#endif
