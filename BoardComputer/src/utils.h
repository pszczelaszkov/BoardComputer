/*
 * utils.c
 *
 * Created: 2020-09-03 03:21:00
 * Author : pszczelaszkov
 */


#ifndef __UTILS__
#define __UTILS__
#ifdef __AVR__
    #include <stdlib.h>
#else
    #define ISR(...) void __VA_ARGS__()
    #define sleep_cpu()
    volatile uint8_t PINA,PINB;
    volatile uint16_t TCNT1;
    uint8_t DDRD, DDRB;
    uint8_t PORTD, PORTB;
    uint8_t ADC;
    uint8_t ADMUX, ADSC, ADCSRA;
    /* reverse:  reverse string s in place */
    void reverse(char s[])
    {
        int i, j;
        char c;

        for (i = 0, j = strlen(s)-1; i<j; i++, j--) {
            c = s[i];
            s[i] = s[j];
            s[j] = c;
        }
    }
    /* itoa:  convert n to characters in s */
    void itoa(int n, char s[],int dummy)
    {
        int i, sign;

        if ((sign = n) < 0)  /* record sign */
            n = -n;          /* make n positive */
        i = 0;
        do {       /* generate digits in reverse order */
            s[i++] = n % 10 + '0';   /* get next digit */
        } while ((n /= 10) > 0);     /* delete it */
        if (sign < 0)
            s[i++] = '-';
        s[i] = '\0';
        reverse(s);
    }
    void _delay_ms(int dummy){}
#endif
#include <string.h>
#include "sensorsfeed.h"
const uint8_t FP8_weight = 10000/0xff;
const uint16_t FP16_weight = SENSORSFEED_HIGH_PRECISION_BASE/0xffff;

//Concatenate right aligned integer.
void rightconcat_short(char* dest, int16_t value, uint8_t spacing)
{
	char temp[6];
	uint8_t length;
	itoa(value, &temp[0],10);
	length = strlen(temp);
	memcpy(&dest[spacing-length],temp,length);
}
//Concatenate right aligned integer limited by n.
void rightnconcat_short(char* dest, int16_t value, uint8_t spacing, uint8_t n)
{
	char temp[6];
	if (n > 5)
		n = 5;
	itoa(value, &temp[0],10);
	memcpy(&dest[spacing-n],temp,n);
}
#endif
