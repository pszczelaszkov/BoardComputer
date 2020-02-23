#ifndef __BITWISE__
#define __BITWISE__
#define BIT0 1
#define BIT1 2
#define BIT2 4
#define BIT3 8
#define BIT4 16
#define BIT5 32
#define BIT6 64
#define BIT7 128

#define TOGGLE(var,mask) var ^= mask
#define SET(var,mask) var |= mask
#define CLEAR(var,mask) var &= ~mask
#define READ(var,mask) var & mask

#endif
