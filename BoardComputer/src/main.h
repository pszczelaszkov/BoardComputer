/*
 * main.c
 *
 * Created: 2022-05-15 02:53:00
 * Author : pszczelaszkov
 */
#include "utils.h"
#ifdef __AVR__
#define ENTRY_ROUTINE void main()
#else
#define ENTRY_ROUTINE TESTUSE void test()
#endif
TESTUSE void core();