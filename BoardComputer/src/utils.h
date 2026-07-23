/*
 * utils.h
 *
 * Created: 2020-09-03 03:21:00
 * Author : pszczelaszkov
 */


#ifndef __UTILS__
#define __UTILS__
#define TESTADDPREFIX(...) __VA_ARGS__
#define TESTSTATICVAR(...) __VA_ARGS__
#define MIN(i, j) (((i) < (j)) ? (i) : (j))
#define MAX(i, j) (((i) > (j)) ? (i) : (j))
#include <stdint.h>
#include <string.h>

TESTUSE int16_t UTILS_atoi(char* stringvalue);
TESTUSE void u16toa(uint16_t n, char s[]);
TESTUSE void i16toa(int16_t n, char s[]);
TESTUSE void i32toa(int32_t n, char s[]);
TESTUSE void extractfp16(int16_t fixedpoint, int8_t* integral, uint16_t* fractional);
TESTUSE void fp16toa(int16_t fixedpoint, char* dest, uint8_t integrallength, uint8_t fractionlength);
TESTUSE typedef void (*Callback)();
TESTUSE typedef void (*Callback_32)(int32_t);
TESTUSE void rightconcat_short(char* dest, int16_t value, uint8_t spacing);
TESTUSE void rightnconcat_short(char* dest, int16_t value, uint8_t spacing, uint8_t n);
#endif
