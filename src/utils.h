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

float distance(int x1, int y1, int x2, int y2);
int roll(int a, int x);

#endif