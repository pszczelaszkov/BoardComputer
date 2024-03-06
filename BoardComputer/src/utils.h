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
#ifdef __AVR__
#include<stdlib.h>
#else
    #include <inttypes.h>
    #define ISR(...) void __VA_ARGS__()
    #define sleep_cpu()
    TESTUSE extern volatile uint8_t PINA, PINB, PINC, PIND;
    TESTUSE extern volatile uint16_t TCNT1, TCNT2;
    TESTUSE extern uint8_t DDRA, DDRB, DDRC, DDRD;
    TESTUSE extern uint8_t PORTA, PORTB, PORTC, PORTD, DIDR0;
    TESTUSE extern uint16_t ADC;
    TESTUSE extern uint8_t ADMUX, ADSC, ADCSRA;
    TESTUSE void ADC_vect();
    /* reverse:  reverse string s in place */
    void reverse(char s[]);
    /* itoa:  convert n to characters in s */
    void itoa(int n, char s[],int dummy);
    void _delay_ms(int dummy);
#endif
#include <string.h>
#include "sensorsfeed.h"
TESTUSE int16_t UTILS_atoi(char* stringvalue);
void uitoa(uint16_t n, char s[]);
void extractfp16(int16_t fixedpoint, int8_t* integral, uint16_t* fractional);
TESTUSE void fp16toa(int16_t fixedpoint, char* dest, uint8_t integrallength, uint8_t fractionlength);
TESTUSE typedef void (*Callback)();
TESTUSE typedef void (*Callback_32)(uint32_t);
TESTUSE void rightconcat_short(char* dest, int16_t value, uint8_t spacing);
TESTUSE void rightnconcat_short(char* dest, int16_t value, uint8_t spacing, uint8_t n);
#endif
