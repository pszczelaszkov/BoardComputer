/*
 * utils.h
 *
 * Created: 2020-09-03 03:21:00
 * Author : pszczelaszkov
 */


#ifndef __UTILS__
#define __UTILS__
#ifdef __AVR__
#include<stdlib.h>
#else
    #include <inttypes.h>
    #define ISR(...) void __VA_ARGS__()
    #define sleep_cpu()
    extern volatile uint8_t PINA,PINB;
    extern volatile uint16_t TCNT1,TCNT2;
    extern uint8_t DDRD, DDRB;
    extern uint8_t PORTD, PORTB, PIND;
    extern uint8_t ADC;
    extern uint8_t ADMUX, ADSC, ADCSRA;
    /* reverse:  reverse string s in place */
    void reverse(char s[]);
    /* itoa:  convert n to characters in s */
     void itoa(int n, char s[],int dummy);
    void _delay_ms(int dummy);
#endif
#include <string.h>
#include "sensorsfeed.h"
void uitoa(uint16_t n, char s[]);
extern const uint8_t FP8_weight;
extern const uint16_t FP16_weight;
typedef void (*Callback)();
typedef void (*Callback_32)(uint32_t);
void rightconcat_short(char* dest, int16_t value, uint8_t spacing);
void rightnconcat_short(char* dest, int16_t value, uint8_t spacing, uint8_t n);
#endif
